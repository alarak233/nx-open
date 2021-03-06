/*****************************************************************************
**
** toolPath.c
**
** Description:
**     Contains Unigraphics entry points for the application.
**
*****************************************************************************/

/* Include files */
#include <stdarg.h>
#include <stdio.h>
#include <uf.h>
#include <uf_ui.h>
#include <uf_modl.h>
#include <uf_curve.h>
#include <math.h>
#include <stdlib.h>

#define  NUMBER_POINTS  5

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

	int response;
	int i = 0;
	int j = 0;
	int k = 0;
	int l = 0;
	int m = 0;

	/*1.选取目标面*/

	tag_t *baseSurfaceTag = NULL_TAG;
	tag_t viewTag = NULL_TAG;
	int faceNum = 0;
	UF_UI_select_with_class_dialog("请选择目标面", "目标面", UF_UI_SEL_SCOPE_ANY_IN_ASSEMBLY, init_proc, NULL, &response, &faceNum, &baseSurfaceTag);

	/*2.选取中心点*/

//	使用了点收集器，应该使用点构造器
/*
	UF_UI_chained_points_t *centralPoint_t[1];
	int centralPointsNum = 1;
	UF_UI_select_point_collection("请选择中心点",false,centralPoint_t,&centralPointsNum,&response);
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

//	创建面上的点所需要的uv参数为0-1，不需要查询面的参数（重构理由见下面）
	double uvMinMax[4];

	char uvNumChar[3][16] = { "u向","v向" ,"刀具圆弧半径" };
	double uvNum[3] = { 51,51,3 };
	uc1609("请选择u向v向点个数", uvNumChar, 3, uvNum, 0);

	/*4.生成点的法向量并对其进行转换*/

	double uvParameter[2];
	double ut, vt;
	double uvPoint[3];
	double uFirstDerivative[3];
	double uSecondDerivative[3];
	double vFirstDerivative[3];
	double vSecondDerivative[3];
	double normalDirection[3];
	double curvatureRadius[2];
	//计算uv与长度比值辅助量，先不算,记录基准值
	double uvBaseParameter[2];
	/*
	double uMaxd = 0;
	double vMaxd = 0;
	*/
	tag_t uvPointTag = NULL_TAG;
	//char uvPointChar[50];
	int isPointOnSurface = 0;

	//可视化用的直线
	UF_CURVE_line_t line;
	tag_t lineTag = NULL_TAG;

	//面上的点与中心连接所在平面法向量及长度
	double toolDirection[3] = { 0.0,0.0,0.0 };
	double toolDirectionLength = 0;

	//法向量在加工面上的投影
	double toolPlaneProjection[3] = { 0.0,0.0,0.0 };

	//法向量在加工面法向量上的投影及投影长度
	double normalVectorProjectionLength = 0;
	double normalVectorProjection[3] = { 0.0,0.0,0.0 };

	//连接同一行的点的辅助变量
	//UF_CURVE_create_spline_thru_pts用到的点结构体
	UF_CURVE_pt_slope_crvatr_t splinePoints[2000];
	//初始化
	for (i = 0; i < 2000; i++)
	{
		splinePoints[i].slope_type = UF_CURVE_SLOPE_NONE;
		splinePoints[i].crvatr_type = UF_CURVE_CRVATR_NONE;
	}
	splinePoints[0].slope_type = UF_CURVE_SLOPE_AUTO;
	//一行在面上的点的个数
	int splinePointNum = 0;

	//曲线个数及标识符；
	//tag_t splineTag[2000];
	int splineNum = 0;

	//测试点
	tag_t pointTag = NULL_TAG;

	//构造曲面用的点链表！！！需要分配内存！！！
	UF_MODL_bsurf_row_info_t *bsurfacePoints;
	bsurfacePoints = (UF_MODL_bsurf_row_info_t *)malloc((int)(uvNum[0] + 1) * (int)sizeof(UF_MODL_bsurf_row_info_t));
	for (i = 0; i <= uvNum[0]; i++)
	{
		bsurfacePoints[i].num_points = (int)uvNum[0];
		bsurfacePoints[i].points = (double *)malloc((int)(uvNum[1] + 1) * 3 * sizeof(double*));
		bsurfacePoints[i].weight = (double *)malloc((int)(uvNum[1] + 1) * sizeof(double*));
	}

	//测试内存分配
	/*
	UF_UI_open_listing_window();
	char test[20];
	sprintf(test, "%zd", sizeof(UF_MODL_bsurf_row_info_t));
	UF_UI_write_listing_window(test);
	*/

	//二分法用的辅助变量
	double minu, minv, midu, midv, maxu, maxv;
	double endu, endv;
	double uvBasePoint[2];

	for (l = 0; l < faceNum; l++)
	{
		UF_MODL_ask_face_uv_minmax(baseSurfaceTag[l], uvMinMax);
		//计算uv与长度的比值，默认1：1000，先不算
		/*
		UF_MODL_ask_face_props(baseSurfaceTag[l], uvParameter, uvPoint, uFirstDerivative, vFirstDerivative, uSecondDerivative, vSecondDerivative, normalDirection, curvatureRadius);
		*/
		//splineNum = 0;
		
		UF_UI_open_listing_window();
		char test[40];
		sprintf(test, "%.2f %.2f %.2f %.2f\n", uvMinMax[0], uvMinMax[1], uvMinMax[2], uvMinMax[3]);
		UF_UI_write_listing_window(test);
		
		for (i = 1; i < uvNum[0]; i++)
		{
			splinePointNum = 0;
			ut = i / uvNum[0];
			ut = ut * 0.9999 + 0.00005;

			for (j = 1; j < uvNum[1]; j++)
			{
				vt = j / uvNum[1];
				vt = vt * 0.9999 + 0.00005;
				uvParameter[0] = (uvMinMax[1] - uvMinMax[0])*ut + uvMinMax[0];
				uvParameter[1] = (uvMinMax[3] - uvMinMax[2])*vt + uvMinMax[2];
				uvBaseParameter[0] = uvParameter[0];
				uvBaseParameter[1] = uvParameter[1];

				//判断点是否在面上
				UF_MODL_ask_face_props(baseSurfaceTag[l], uvParameter, uvPoint, uFirstDerivative, vFirstDerivative, uSecondDerivative, vSecondDerivative, normalDirection, curvatureRadius);
				UF_MODL_ask_point_containment(uvPoint, baseSurfaceTag[l], &isPointOnSurface);
				uvBasePoint[0] = uvPoint[0];
				uvBasePoint[1] = uvPoint[1];
				if (isPointOnSurface == 1)
				{			
					//计算面上的点与中心连接所在平面的法向量
					toolDirection[0] = uvPoint[1] - centralPoint[1];
					toolDirection[1] = centralPoint[0] - uvPoint[0];

					//计算空间法向量在加工平面上的投影向量
					//先计算法向量在平面法向量上的投影(toolDirection[2]=0,可以手动优化)
					toolDirectionLength = sqrt(pow(toolDirection[0], 2) + pow(toolDirection[1], 2) + pow(toolDirection[2], 2));
					normalVectorProjectionLength = (toolDirection[0] * normalDirection[0] + toolDirection[1] * normalDirection[1] + toolDirection[2] * normalDirection[2]) / toolDirectionLength;
					normalVectorProjection[0] = toolDirection[0] * normalVectorProjectionLength / toolDirectionLength;
					normalVectorProjection[1] = toolDirection[1] * normalVectorProjectionLength / toolDirectionLength;
					normalVectorProjection[2] = toolDirection[2] * normalVectorProjectionLength / toolDirectionLength;
					//然后计算在加工平面上的法向量
					toolPlaneProjection[0] = normalDirection[0] - normalVectorProjection[0];
					toolPlaneProjection[1] = normalDirection[1] - normalVectorProjection[1];
					toolPlaneProjection[2] = normalDirection[2] - normalVectorProjection[2];
					//根据刀具半径改变向量大小
					if (toolPlaneProjection[2] < 0)
					{
						toolPlaneProjection[0] = -toolPlaneProjection[0];
						toolPlaneProjection[1] = -toolPlaneProjection[1];
						toolPlaneProjection[2] = -toolPlaneProjection[2];
					}
					toolDirectionLength = sqrt(pow(toolPlaneProjection[0], 2) + pow(toolPlaneProjection[1], 2) + pow(toolPlaneProjection[2], 2));
					toolPlaneProjection[0] = toolPlaneProjection[0] * uvNum[2] / toolDirectionLength;
					toolPlaneProjection[1] = toolPlaneProjection[1] * uvNum[2] / toolDirectionLength;
					toolPlaneProjection[2] = toolPlaneProjection[2] * uvNum[2] / toolDirectionLength;
					//根据法向量正负计算min mid max u v	
					if (toolPlaneProjection[0] > 0)
					{
						midu = uvPoint[0] - toolPlaneProjection[0];
						maxu = uvPoint[0];
						minu = uvPoint[0] - 2*toolPlaneProjection[0];
					}
					else
					{
						midu = uvPoint[0] + toolPlaneProjection[0];
						minu = uvPoint[0];
						maxu = uvPoint[0] + 2 * toolPlaneProjection[0];
					}
					if (toolPlaneProjection[1] > 0)
					{
						midv = uvPoint[1] - toolPlaneProjection[1];
						maxv = uvPoint[1];
						minv = uvPoint[1] -2* toolPlaneProjection[1];
					}
					else
					{
						midv = uvPoint[1] - toolPlaneProjection[1];
						maxv = uvPoint[1];
						maxv = uvPoint[1] - 2 * toolPlaneProjection[1];
					}
					//计算参数
					uvParameter[0] = (midu- uvBasePoint[0]) / 1000.0+uvBaseParameter[0];
					uvParameter[1] = (midv - uvBasePoint[1]) / 1000.0 + uvBaseParameter[1];

					//尝试开始做补偿面极点优化以防止补偿面中心褶皱
					for (m = 0; m < 1; m++)
					{
						//判断点是否在面上
						UF_MODL_ask_face_props(baseSurfaceTag[l], uvParameter, uvPoint, uFirstDerivative, vFirstDerivative, uSecondDerivative, vSecondDerivative, normalDirection, curvatureRadius);
						UF_MODL_ask_point_containment(uvPoint, baseSurfaceTag[l], &isPointOnSurface);

						if (isPointOnSurface == 1)
						{
							//计算面上的点与中心连接所在平面的法向量
							toolDirection[0] = uvPoint[1] - centralPoint[1];
							toolDirection[1] = centralPoint[0] - uvPoint[0];

							//计算空间法向量在加工平面上的投影向量
							//先计算法向量在平面法向量上的投影(toolDirection[2]=0,可以手动优化)
							toolDirectionLength = sqrt(pow(toolDirection[0], 2) + pow(toolDirection[1], 2) + pow(toolDirection[2], 2));
							normalVectorProjectionLength = (toolDirection[0] * normalDirection[0] + toolDirection[1] * normalDirection[1] + toolDirection[2] * normalDirection[2]) / toolDirectionLength;
							normalVectorProjection[0] = toolDirection[0] * normalVectorProjectionLength / toolDirectionLength;
							normalVectorProjection[1] = toolDirection[1] * normalVectorProjectionLength / toolDirectionLength;
							normalVectorProjection[2] = toolDirection[2] * normalVectorProjectionLength / toolDirectionLength;
							//然后计算在加工平面上的法向量
							toolPlaneProjection[0] = normalDirection[0] - normalVectorProjection[0];
							toolPlaneProjection[1] = normalDirection[1] - normalVectorProjection[1];
							toolPlaneProjection[2] = normalDirection[2] - normalVectorProjection[2];
							//确定法向方向
							if (toolPlaneProjection[2] < 0)
							{
								toolPlaneProjection[0] = -toolPlaneProjection[0];
								toolPlaneProjection[1] = -toolPlaneProjection[1];
								toolPlaneProjection[2] = -toolPlaneProjection[2];
							}
							//根据刀具半径改变向量大小
							toolDirectionLength = sqrt(pow(toolPlaneProjection[0], 2) + pow(toolPlaneProjection[1], 2) + pow(toolPlaneProjection[2], 2));
							toolPlaneProjection[0] = toolPlaneProjection[0] * uvNum[2] / toolDirectionLength;
							toolPlaneProjection[1] = toolPlaneProjection[1] * uvNum[2] / toolDirectionLength;
							toolPlaneProjection[2] = toolPlaneProjection[2] * uvNum[2] / toolDirectionLength;

							//根据二分法计算min mid max u v	
							endu = uvPoint[0] + toolPlaneProjection[0];
							endv = uvPoint[1] + toolPlaneProjection[1];
							if (endu >= uvBasePoint[0])
							{
								maxu = midu;
								midu = minu+0.5*(maxu-minu);
							}
							else
							{
								minu= midu;
								midu = minu + 0.5*(maxu - minu);
							}
							if (endv >= uvBasePoint[1])
							{
								maxv = midv;
								midv = minv + 0.5*(maxv - minv);
							}
							else
							{
								minv = midv;
								midv = minv + 0.5*(maxv - minv);
							}
							//计算参数
							uvParameter[0] = (midu - uvBasePoint[0]) / 1000.0 + uvBaseParameter[0];
							uvParameter[1] = (midv - uvBasePoint[1]) / 1000.0 + uvBaseParameter[1];

							//测试
							/*
							line.end_point[0] = uvPoint[0] + toolPlaneProjection[0];
							line.end_point[1] = uvPoint[1] + toolPlaneProjection[1];
							line.end_point[2] = uvPoint[2] + toolPlaneProjection[2];
							UF_CURVE_create_point(line.end_point, &pointTag);
							*/
						}
						else
						{
							break;
						}
					}
				}

				if (isPointOnSurface == 1)
				{
					//sprintf(uvPointChar, "%.4f %.4f %.4f %.4f \n", uvPoint[0], uvPoint[1], uvPoint[2], clearanceAngle);//输出点坐标和点的后角
					//UF_CURVE_create_point(uvPoint, &uvPointTag);

					//后角可视化
					line.start_point[0] = uvPoint[0];
					line.start_point[1] = uvPoint[1];
					line.start_point[2] = uvPoint[2];
					line.end_point[0] = uvPoint[0] + toolPlaneProjection[0];
					line.end_point[1] = uvPoint[1] + toolPlaneProjection[1];
					line.end_point[2] = uvPoint[2] + toolPlaneProjection[2];
					UF_CURVE_create_line(&line, &lineTag);

					//记录点坐标
					splinePoints[splinePointNum].point[0] = line.end_point[0];
					splinePoints[splinePointNum].point[1] = line.end_point[1];
					splinePoints[splinePointNum].point[2] = line.end_point[2];

					//曲率方向
					//splinePoints[splinePointNum].crvatr[0] = -toolPlaneProjection[0];
					//splinePoints[splinePointNum].crvatr[1] = -toolPlaneProjection[1];
					//splinePoints[splinePointNum].crvatr[2] = -toolPlaneProjection[2];

					//测试输出点
					UF_CURVE_create_point(splinePoints[splinePointNum].point, &pointTag);

					//记录点集，注意为一维数组
					bsurfacePoints[splineNum].points[3 * splinePointNum] = line.end_point[0];
					bsurfacePoints[splineNum].points[3 * splinePointNum + 1] = line.end_point[1];
					bsurfacePoints[splineNum].points[3 * splinePointNum + 2] = line.end_point[2];
					bsurfacePoints[splineNum].weight[splinePointNum] = 1;

					//点计数
					splinePointNum++;
				}
			}
			if (splinePointNum > 1)
			{
				splinePoints[splinePointNum].slope_type = UF_CURVE_SLOPE_AUTO;
				//UF_CURVE_create_spline_thru_pts(3, 0, splinePointNum, splinePoints, NULL, 1, &(splineTag[splineNum]));
				bsurfacePoints[splineNum].num_points = splinePointNum;
				splineNum++;
				splinePoints[splinePointNum].slope_type = UF_CURVE_SLOPE_NONE;

			}
		}
	}

	//测试
	/*
	UF_UI_open_listing_window();
	char test[20];
	sprintf(test, "%d", splineNum);
	UF_UI_write_listing_window(test);
	*/
	/*
	//定义曲线集合及初始化，为构造曲面做准备
	UF_STRING_t splineList;
	UF_STRING_t spineList;
	UF_MODL_init_string_list(&splineList);
	//UF_MODL_init_string_list(&spineList);
	UF_MODL_create_string_list(splineNum, splineNum, &splineList);
	//UF_MODL_create_string_list(splineNum, splineNum, &spineList);
	splineList.num = splineNum;
	tag_t *splineFeatures = 0;
	int splineFeaturesNum = 0;
	for (i = 0; i < splineNum; i++)
	{
		splineList.string[i] = 1;
		splineList.dir[i] = UF_MODL_CURVE_START_FROM_BEGIN;
		//UF_CURVE_ask_feature_curves(splineTag[i], &splineFeaturesNum, &splineFeatures);
		splineList.id[i] = splineTag[i];
	}
	int patch = 2;
	int alignment = 1;
	double value[6];
	int vdegreee = 3;
	int vstatus = 0;
	int bodytype = 1;
	double tolerance[3] = { 0.000011,0.0001,0.1 };
	tag_t neighborSurface[2] = { NULL_TAG,NULL_TAG };
	int constraint[2] = { 0,0 };
	tag_t finalSurface = NULL_TAG;

	//创建网格曲面
	UF_MODL_create_thru_curves(&splineList, &spineList, &patch, &alignment, value, &vdegreee, &vstatus, &bodytype, UF_NULLSIGN, tolerance, neighborSurface, constraint, &finalSurface);
	UF_MODL_free_string_list(&splineList);
	//UF_MODL_free_string_list(&spineList);
	*/
	//尝试用点构造曲面
	//辅助变量
	tag_t bsurfaceTag = NULL_TAG;
	//构造
	UF_MODL_create_bsurf_thru_pts(1, 0, 0, 3, 3, splineNum, bsurfacePoints, &bsurfaceTag);

	//释放内存
	for (i = 0; i <= uvNum[0]; i++)
	{
		free(bsurfacePoints[i].points);
		free(bsurfacePoints[i].weight);
	}
	free(bsurfacePoints);

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

