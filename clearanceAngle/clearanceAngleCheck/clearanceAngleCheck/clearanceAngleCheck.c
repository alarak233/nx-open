/*****************************************************************************
**
** clearanceAngleCheck.c
**
** Description:
**     Contains Unigraphics entry points for the application.
**
*****************************************************************************/

/* Include files */
#include <stdarg.h>
#include <stdio.h>
#include <uf.h>
#include <uf_ui_types.h>
#include <uf_ui.h>
#include <uf_modl.h>
#include <uf_so.h>
#include <uf_point.h>
#include <uf_curve.h>
#include <uf_obj.h>
#include <math.h>

static void ECHO(char *format, ...)
{
	char msg[UF_UI_MAX_STRING_BUFSIZE];
	va_list args;
	va_start(args, format);
	vsnprintf_s(msg, sizeof(msg), _TRUNCATE, format, args);
	va_end(args);
	UF_UI_open_listing_window();
	UF_UI_write_listing_window(msg);
	UF_print_syslog(msg, FALSE);
}

#define UF_CALL(X) (report_error( __FILE__, __LINE__, #X, (X)))

static int report_error(char *file, int line, char *call, int irc)
{
	if (irc)
	{
		char err[133];

		UF_get_fail_message(irc, err);
		ECHO("*** ERROR code %d at line %d in %s:\n",
			irc, line, file);
		ECHO("+++ %s\n", err);
		ECHO("%s;\n", call);
	}

	return(irc);
}
/*选取特定的特征过滤器初始化*/
static int init_proc(UF_UI_selection_p_t select, void *user_data)
{
	int  errorCode = 0;
	int numTriples = 1;
	UF_UI_mask_t mask_triples[] = { UF_face_type,0,0 };
	errorCode = UF_UI_set_sel_mask(select, UF_UI_SEL_MASK_CLEAR_AND_ENABLE_SPECIFIC, numTriples, mask_triples);
	if (errorCode == 0)
	{
		return UF_UI_SEL_SUCCESS;
	}
	else
	{
		return UF_UI_SEL_FAILURE;
	}
}

/*****************************************************************************
**  Activation Methods
*****************************************************************************/
/*  Explicit Activation
**      This entry point is used to activate the application explicitly, as in
**      "File->Execute UG/Open->User Function..." */
extern DllExport void ufusr(char *parm, int *returnCode, int rlen)
{
	/* Initialize the API environment */
	if (UF_CALL(UF_initialize()))
	{
		/* Failed to initialize */
		return;
	}

	/* TODO: Add your application code here */

	/*全局变量*/
	//错误返回
	int response = 0;

	int i = 0;
	int j = 0;
	int k = 0;
	int l = 0;

	int clearanceAngleColor = 186;//red
	int radiaAngleColor = 211;//blue

	/*1.选取目标面*///尝试选择多个面

	tag_t *baseSurfaceTag = NULL_TAG;
	tag_t viewTag = NULL_TAG;
	double cursor[3] = { 0.0,0.0,0.0 };
	int faceNum = 0;

	//选取单个面
	//UF_UI_select_with_single_dialog("请选择目标面", "目标面", UF_UI_SEL_SCOPE_ANY_IN_ASSEMBLY, init_proc, NULL, &response, &baseSurfaceTag[0], cursor, &viewTag);

	//选取多个面
	UF_UI_select_with_class_dialog("请选择目标面", "目标面", UF_UI_SEL_SCOPE_ANY_IN_ASSEMBLY, init_proc, NULL, &response, &faceNum, &baseSurfaceTag);

	/*2.选取中心点*/

//	使用了点收集器，应该使用点构造器
/*
	UF_UI_chained_points_t *centralPoint_t[1];
	int centralPointsNum = 1;
	UF_UI_select_point_collection("请选择中心点",false,centralPoint_t,&centralPointsNum,&response);

//测试
	UF_UI_open_listing_window();
	char pointCoordinates[20];
	sprintf(pointCoordinates, "%.3f %.3f %.3f\n", centralPoint_t[0]->pt[0], centralPoint_t[0]->pt[1], centralPoint_t[0]->pt[2]);
	UF_UI_write_listing_window(pointCoordinates);
*/

	UF_UI_POINT_base_method_t pointConstrustMethod = UF_UI_POINT_NO_METHOD;
	tag_t centralPointTag;
	double centralPoint[3] = { 0.0,0.0,0.0 };
	UF_UI_point_construct("请选择中心点", &pointConstrustMethod, &centralPointTag, centralPoint, &response);

	//测试
	/*
		UF_UI_open_listing_window();
		char pointCoordinates[20];
		sprintf(pointCoordinates, "%.3f %.3f %.3f\n",centralPoint[0],centralPoint[1],centralPoint[2]);
		UF_UI_write_listing_window(pointCoordinates);
	*/

	/*3.选取切削方向（顺时针，逆时针）及点的分布（uv点数）*/

/*	直接导出顺逆时针的两个值，不用选择了
	char cuttingDirection[2][38] = { "顺时针","逆时针" };
	int isClockwise = 5;
	isClockwise = uc1603("请选择切削方向", 0, cuttingDirection, 2);
*/

    //创建对话框
	double uvMinMax[4];
	char uvNumChar[2][16] = { "u向","v向" };
	double uvNum[2] = { 51,51 };
	uc1609("请选择u向v向点个数", uvNumChar, 2, uvNum, 0);

	//变量声明
	//参数
	double uvParameter[2];
	double ut, vt;
	double uvPoint[3];
	double uFirstDerivative[3];
	double uSecondDerivative[3];
	double vFirstDerivative[3];
	double vSecondDerivative[3];
	double normalDirection[3];
	double curvatureRadius[2];
	tag_t uvPointTag = NULL_TAG;
	char uvPointChar[50];
	int isPointOnSurface = 0;

	//可视化用的直线
	UF_CURVE_line_t line;
	tag_t lineTag = NULL_TAG;

	//面上的点与中心连接所在平面法向量
	double toolDirection[3] = { 0.0,0.0,0.0 };

	//后角
	double clearanceAngle;
	double maxClearanceAngle = 0;
	double minClearanceAngle = 0;
	double maxClearanceAnglePoint[3] = { 0.0,0.0,0.0 };
	double minClearanceAnglePoint[3] = { 0.0,0.0,0.0 };
	tag_t maxClearanceAnglePointTag = NULL_TAG;
	tag_t minClearanceAnglePointTag = NULL_TAG;

	//向量积及法向斜率
	double vectorProduct[3] = { 0.0,0.0,0.0 };
	double radialAngle = 0.0;
	double radialSlope = 0.0;
	double maxRadiaAngle = 0.0;
	double minRadiaAngle = 0.0;
	double maxRadiaAnglePoint[3] = { 0.0,0.0,0.0 };
	double minRadiaAnglePoint[3] = { 0.0,0.0,0.0 };
	tag_t maxRadiaAnglePointTag = NULL_TAG;
	tag_t minRadiaAnglePointTag = NULL_TAG;

	//计算最大范围辅助变量
	double maxd = 0.0;
	double maxdSum = 0;
	double uvMinParameter[2] = { 0.0,0.0 };
	double uvMaxParameter[2] = { 0.0,0.0 };
	double uvmin[2] = { 0.0,0.0 };
	double uvmax[2] = { 0.0,0.0 };
	int uvminOrder = 0;
	int uvmaxOrder = 0;

	for (l = 0; l < faceNum; l++)
	{
		//可视化中求选取面的最大范围
		UF_MODL_ask_face_uv_minmax(baseSurfaceTag[l], uvMinMax);
		if (uvMinParameter[0] > uvMinMax[0] || uvMinParameter[1] > uvMinMax[2])
		{
			uvMinParameter[0] = uvMinMax[0];
			uvMinParameter[1] = uvMinMax[2];
			uvminOrder = l;
		}
		if (uvMaxParameter[0] < uvMinMax[1] || uvMaxParameter[1] < uvMinMax[3])
		{
			uvMaxParameter[0] = uvMinMax[1];
			uvMaxParameter[1] = uvMinMax[3];
			uvmaxOrder = l;
		}
	}
	UF_MODL_ask_face_props(baseSurfaceTag[uvminOrder], uvMinParameter, uvPoint, uFirstDerivative, vFirstDerivative, uSecondDerivative, vSecondDerivative, normalDirection, curvatureRadius);
	uvmin[0] = uvPoint[0];
	uvmin[1] = uvPoint[1];

	UF_MODL_ask_face_props(baseSurfaceTag[uvmaxOrder], uvMaxParameter, uvPoint, uFirstDerivative, vFirstDerivative, uSecondDerivative, vSecondDerivative, normalDirection, curvatureRadius);
	uvmax[0] = uvPoint[0];
	uvmax[1] = uvPoint[1];
	maxd = sqrt(pow((uvmax[0] - uvmin[0]), 2) + pow((uvmax[0] - uvmin[0]), 2));
	maxdSum = maxd;


	for (l = 0; l < faceNum; l++)
	{

		UF_MODL_ask_face_uv_minmax(baseSurfaceTag[l], uvMinMax);


		//	测试对话框是否有效
		/*
			UF_UI_open_listing_window();

			char isClockwiseChar[5];
			sprintf(isClockwiseChar, "%d \n", isClockwise);
			UF_UI_write_listing_window(isClockwiseChar);

			sprintf(uvNumChar[0], "%.0f %.0f\n", uvNum[0], uvNum[1]);
			UF_UI_write_listing_window(uvNumChar[0]);

			char uvMinMaxChar[50];
			sprintf(uvMinMaxChar, "%.3f %.3f %.3f %.3f\n",uvMinMax[0],uvMinMax[1],uvMinMax[2],uvMinMax[3]);
			UF_UI_write_listing_window(uvMinMaxChar);
		*/

		//	测试使用point create on surface函数，生成点太多可能会卡，尝试使用ask point on surface 重构
		/*
			tag_t uTag = NULL_TAG;
			tag_t vTag = NULL_TAG;
			UF_SO_create_scalar_double(baseSurfaceTag[l], UF_SO_update_within_modeling, 0.2, &uTag);
			UF_SO_create_scalar_double(baseSurfaceTag[l], UF_SO_update_within_modeling, 0.2, &vTag);

			UF_UI_open_listing_window();

			double testUV = 0;
			UF_SO_ask_double_of_scalar(uTag, &testUV);
			char testUVChar[10];
			sprintf(testUVChar, "%.4f \n", testUV);
			UF_UI_write_listing_window(testUVChar);


			tag_t testPoint = NULL_TAG;
			UF_POINT_create_on_surface(baseSurfaceTag[l], uTag , vTag , &testPoint);
		*/
		/*4.生成点的法向量并对其进行分解得到后角值得到最大后角*/

		UF_UI_open_listing_window();

		for (i = 0; i <= uvNum[0]; i++)
		{
			ut = i / uvNum[0];
			for (j = 0; j <= uvNum[1]; j++)
			{
				vt = j / uvNum[1];
				ut = ut * 0.999 + 0.0005;
				vt = vt * 0.999 + 0.0005;
				uvParameter[0] = (uvMinMax[1] - uvMinMax[0])*ut + uvMinMax[0];
				uvParameter[1] = (uvMinMax[3] - uvMinMax[2])*vt + uvMinMax[2];
				UF_MODL_ask_face_props(baseSurfaceTag[l], uvParameter, uvPoint, uFirstDerivative, vFirstDerivative, uSecondDerivative, vSecondDerivative, normalDirection, curvatureRadius);

				//判断点是否在面上
				UF_MODL_ask_point_containment(uvPoint, baseSurfaceTag[l], &isPointOnSurface);

				if (isPointOnSurface == 1)
				{
					//计算面上的点与中心连接所在平面的法向量
					toolDirection[0] = uvPoint[1] - centralPoint[1];
					toolDirection[1] = centralPoint[0] - uvPoint[0];

					//计算后角
					clearanceAngle = acos((toolDirection[0] * normalDirection[0] + toolDirection[1] * normalDirection[1] + toolDirection[2] * normalDirection[2]) / (sqrt(pow(toolDirection[0], 2) + pow(toolDirection[1], 2) + pow(toolDirection[2], 2))*sqrt(pow(normalDirection[0], 2) + pow(normalDirection[1], 2) + pow(normalDirection[2], 2))));
					//clearanceAngle = clearanceAngle > PI / 2 ? clearanceAngle : PI-clearanceAngle ;
					clearanceAngle = (clearanceAngle - PI / 2) / PI * 180;

					//测试输出法向量(结果法向量均为单位向量)
					//sprintf(uvPointChar, "%.6f %.6f %.6f\n", normalDirection[0], normalDirection[1], normalDirection[2]);
					//UF_UI_write_listing_window(uvPointChar);

					if (clearanceAngle > maxClearanceAngle)
					{
						maxClearanceAngle = clearanceAngle;
						maxClearanceAnglePoint[0] = uvPoint[0];
						maxClearanceAnglePoint[1] = uvPoint[1];
						maxClearanceAnglePoint[2] = uvPoint[2];
					}

					if (clearanceAngle < minClearanceAngle)
					{
						minClearanceAngle = clearanceAngle;
						minClearanceAnglePoint[0] = uvPoint[0];
						minClearanceAnglePoint[1] = uvPoint[1];
						minClearanceAnglePoint[2] = uvPoint[2];
					}

					//sprintf(uvPointChar, "%.4f %.4f %.4f %.4f \n", uvPoint[0], uvPoint[1], uvPoint[2], clearanceAngle);//输出点坐标和点的后角
					//UF_CURVE_create_point(uvPoint, &uvPointTag);

					//后角可视化
					line.start_point[0] = uvPoint[0] + 6 * maxdSum;
					line.start_point[1] = uvPoint[1];
					line.start_point[2] = 0;
					line.end_point[0] = uvPoint[0] + 6 * maxdSum;
					line.end_point[1] = uvPoint[1];
					line.end_point[2] = clearanceAngle;
					UF_CURVE_create_line(&line, &lineTag);
					UF_OBJ_set_color(lineTag, clearanceAngleColor);

					//计算径向斜率
					//计算向量积
					vectorProduct[0] = normalDirection[1] * toolDirection[2] - normalDirection[2] * toolDirection[1];
					vectorProduct[1] = normalDirection[2] * toolDirection[0] - normalDirection[0] * toolDirection[2];
					vectorProduct[2] = normalDirection[0] * toolDirection[1] - normalDirection[1] * toolDirection[0];

					//计算向量积与z轴夹角
					radialAngle = acos(vectorProduct[2] / sqrt(pow(vectorProduct[0], 2) + pow(vectorProduct[1], 2) + pow(vectorProduct[2], 2)));
					radialAngle = PI / 2 - radialAngle;
					radialAngle = radialAngle / PI * 180;

					if (radialAngle > maxRadiaAngle)
					{
						maxRadiaAngle = radialAngle;
						maxRadiaAnglePoint[0] = uvPoint[0];
						maxRadiaAnglePoint[1] = uvPoint[1];
						maxRadiaAnglePoint[2] = uvPoint[2];
					}

					if (radialAngle < minRadiaAngle)
					{
						minRadiaAngle = radialAngle;
						minRadiaAnglePoint[0] = uvPoint[0];
						minRadiaAnglePoint[1] = uvPoint[1];
						minRadiaAnglePoint[2] = uvPoint[2];
					}

					//周向斜率可视化
					line.start_point[0] = uvPoint[0] + 4 * maxdSum;
					line.start_point[1] = uvPoint[1];
					line.start_point[2] = 0;
					line.end_point[0] = uvPoint[0] + 4 * maxdSum;
					line.end_point[1] = uvPoint[1];
					line.end_point[2] = radialAngle;
					UF_CURVE_create_line(&line, &lineTag);
					UF_OBJ_set_color(lineTag, radiaAngleColor);

				}
			}
		}
	}
	
	sprintf(uvPointChar, "max clearance angle is %.4f  \n", maxClearanceAngle);
	UF_UI_write_listing_window(uvPointChar);
	sprintf(uvPointChar, "min clearance angle is %.4f  \n", minClearanceAngle);
	UF_UI_write_listing_window(uvPointChar);
	
	sprintf(uvPointChar, "max radial angle is %.4f  \n", maxRadiaAngle);
	UF_UI_write_listing_window(uvPointChar);
	sprintf(uvPointChar, "min radial angle is %.4f  \n", minRadiaAngle);
	UF_UI_write_listing_window(uvPointChar);
	
	UF_CURVE_create_point(maxClearanceAnglePoint, &maxClearanceAnglePointTag);
	UF_OBJ_set_color(maxClearanceAnglePointTag, clearanceAngleColor);
	UF_CURVE_create_point(minClearanceAnglePoint, &minClearanceAnglePointTag);
	UF_OBJ_set_color(minClearanceAnglePointTag, clearanceAngleColor);

	UF_CURVE_create_point(maxRadiaAnglePoint, &maxRadiaAnglePointTag);
	UF_OBJ_set_color(maxRadiaAnglePointTag, radiaAngleColor);
	UF_CURVE_create_point(minRadiaAnglePoint, &minRadiaAnglePointTag);
	UF_OBJ_set_color(minRadiaAnglePointTag, radiaAngleColor);
	

	/*5.后角可视化（生成对应点的后角生成的直线集合）（已经整合在上面的模块）*/

	/*6.额外功能，添加计算周向斜率以及径向斜率，周向斜率即后角，所以不计算*/
	//由于uv向不一定是xy方向，所以不采用UF_MODL_ask_face_props生成的uv一二阶导数，以继承在循环内

	/*7.额外功能，添加计算允许刀的最大圆弧半径*/
	//暂时不会用曲面的uv一二阶导数求曲率，只能暂时用平面切出的曲线求曲率并且通过主法线判断凹或凸

	//基准平面法向量
	double normalvector[3] = { 0.0,0.0,0.0 };
	tag_t datumPlaneTag = NULL_TAG;
	tag_t intersectionTag = NULL_TAG;

	//相交曲线上的参数
	double intersectionParameter = 0.0;
	double intersectionPoint[3] = { 0.0,0.0,0.0 };
	double intersectionTangent[3] = { 0.0,0.0,0.0 };
	double intersectionPrincipalNormal[3] = { 0.0,0.0,0.0 };
	double intersectionBinormal[3] = { 0.0,0.0,0.0 };
	double intersectionTorsion = 0.0;
	double intersectionRadious = 0.0;
	double minRadious = 10000.0;
	double minRadiousPoint[3] = { 0.0,0.0,0.0 };
	tag_t minRadiousPointTag = NULL_TAG;

	//曲线特征转对象
	int curvesNum = 0;
	tag_t *curvesTag;

	//删除数组记录的id
	tag_t deleteTag[10000];

	for (i = 0; i < uvNum[0]; i++)
	{
		//创建基准平面
		normalvector[0] = sin(i / uvNum[0] * PI);
		normalvector[1] = cos(i / uvNum[0] * PI);
		UF_MODL_create_fixed_dplane(centralPoint, normalvector, &datumPlaneTag);

		//求交线
		UF_CURVE_create_int_object(faceNum, baseSurfaceTag, 1, &datumPlaneTag, &intersectionTag);

		//记录要删除的tag
		deleteTag[i] = intersectionTag;
		deleteTag[i + (int)uvNum[0]] = datumPlaneTag;

		//求最小曲率半径
		for (j = 0; j <= uvNum[1]; j++)
		{
			intersectionParameter = j / (uvNum[1]);

			//！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！
			//类似与grip解组，要把曲线特征变为曲线对象，非常重要！！！！！！！！！！！
			UF_CURVE_ask_feature_curves(intersectionTag, &curvesNum, &curvesTag);
			//！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！

			for (k = 0; k < curvesNum; k++)
			{

				UF_MODL_ask_curve_props(curvesTag[k], intersectionParameter, intersectionPoint, intersectionTangent, intersectionPrincipalNormal, intersectionBinormal, &intersectionTorsion, &intersectionRadious);

				//判断是否是凹的
				if (intersectionPrincipalNormal[2] >= 0)
				{
					if (intersectionRadious < minRadious)
					{
						minRadious = intersectionRadious;
						minRadiousPoint[0] = intersectionPoint[0];
						minRadiousPoint[1] = intersectionPoint[1];
						minRadiousPoint[2] = intersectionPoint[2];
					}

					//正向可视化
					line.start_point[0] = intersectionPoint[0] + 2 * maxdSum;
					line.start_point[1] = intersectionPoint[1];
					line.start_point[2] = 0;
					line.end_point[0] = intersectionPoint[0] + 2 * maxdSum;
					line.end_point[1] = intersectionPoint[1];
					line.end_point[2] = 1000.0 / intersectionRadious;
					UF_CURVE_create_line(&line, &lineTag);
				}
				else
				{
					//负向可视化
					line.start_point[0] = intersectionPoint[0] + 2 * maxdSum;
					line.start_point[1] = intersectionPoint[1];
					line.start_point[2] = 0;
					line.end_point[0] = intersectionPoint[0] + 2 * maxdSum;
					line.end_point[1] = intersectionPoint[1];
					line.end_point[2] = -1000.0 / intersectionRadious;
					UF_CURVE_create_line(&line, &lineTag);
				}
			}

			UF_free(curvesTag);

			//测试用
			//sprintf(uvPointChar, "min radious is %.4f  \n", intersectionRadious);
			//UF_UI_write_listing_window(uvPointChar);

		}
	}

	//删除辅助线辅助面
	//UF_OBJ_delete_object(intersectionTag);
	int *status;
	UF_OBJ_delete_array_of_objects((int)(2 * uvNum[0]), deleteTag, &status);

	
	if (minRadious != 10000)
	{
		sprintf(uvPointChar, "min radious is %.4f  \n", minRadious);
		UF_UI_write_listing_window(uvPointChar);
		UF_CURVE_create_point(minRadiousPoint, &minRadiousPointTag);
	}
	else
	{
		sprintf(uvPointChar, "min radious is too big");
		UF_UI_write_listing_window(uvPointChar);
	}

	/* Terminate the API environment */
	UF_CALL(UF_terminate());
}

/*****************************************************************************
**  Utilities
*****************************************************************************/

/* Unload Handler
**     This function specifies when to unload your application from Unigraphics.
**     If your application registers a callback (from a MenuScript item or a
**     User Defined Object for example), this function MUST return
**     "UF_UNLOAD_UG_TERMINATE". */
extern int ufusr_ask_unload(void)
{
	return(UF_UNLOAD_IMMEDIATELY);
}

