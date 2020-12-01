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
#include <uf_ui_types.h>
#include <uf_ui.h>
#include <uf_modl.h>
#include <uf_so.h>
#include <uf_point.h>
#include <uf_curve.h>
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

static int report_error( char *file, int line, char *call, int irc)
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
extern DllExport void ufusr( char *parm, int *returnCode, int rlen )
{
    /* Initialize the API environment */
    if( UF_CALL(UF_initialize()) ) 
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

	/*1.选取目标面*/

	tag_t baseSurfaceTag = NULL_TAG;
	tag_t viewTag = NULL_TAG;
	double cursor[3] = { 0.0,0.0,0.0 };
	UF_UI_select_with_single_dialog("请选择目标面", "目标面", UF_UI_SEL_SCOPE_ANY_IN_ASSEMBLY, init_proc, NULL, &response, &baseSurfaceTag, cursor, &viewTag);

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
	UF_MODL_ask_face_uv_minmax(baseSurfaceTag, uvMinMax);

	char uvNumChar[2][16] = { "u向","v向" };
	double uvNum[2] = { 51,51 };
	uc1609("请选择u向v向点个数", uvNumChar, 2, uvNum, 0);

	/*4.生成点的法向量并对其进行分解得到后角值得到最大后角*/

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

	UF_UI_open_listing_window();

	for (i = 0; i <= uvNum[0]; i++)
	{
		ut = i / uvNum[0];
		for (j = 0; j <= uvNum[1]; j++)
		{
			vt = j / uvNum[1];
			uvParameter[0] = (uvMinMax[1] - uvMinMax[0])*ut + uvMinMax[0];
			uvParameter[1] = (uvMinMax[3] - uvMinMax[2])*vt + uvMinMax[2];
			UF_MODL_ask_face_props(baseSurfaceTag, uvParameter, uvPoint, uFirstDerivative, vFirstDerivative, uSecondDerivative, vSecondDerivative, normalDirection, curvatureRadius);

			//判断点是否在面上
			UF_MODL_ask_point_containment(uvPoint, baseSurfaceTag, &isPointOnSurface);

			if (isPointOnSurface == 1)
			{
				//计算面上的点与中心连接所在平面的法向量
				toolDirection[0] = uvPoint[1] - centralPoint[1];
				toolDirection[1] = centralPoint[0] - uvPoint[0];

				//计算后角
				clearanceAngle = acos((toolDirection[0] * normalDirection[0] + toolDirection[1] * normalDirection[1] + toolDirection[2] * normalDirection[2]) / (sqrt(pow(toolDirection[0], 2) + pow(toolDirection[1], 2) + pow(toolDirection[2], 2))*sqrt(pow(normalDirection[0], 2) + pow(normalDirection[1], 2) + pow(normalDirection[2], 2))));
				//clearanceAngle = clearanceAngle > PI / 2 ? clearanceAngle : PI-clearanceAngle ;
				clearanceAngle = (clearanceAngle - PI / 2) / PI * 180;

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
				line.start_point[0] = uvPoint[0];
				line.start_point[1] = uvPoint[1];
				line.start_point[2] = -50;
				line.end_point[0] = uvPoint[0];
				line.end_point[1] = uvPoint[1];
				line.end_point[2] = -50 + clearanceAngle * 10;
				UF_CURVE_create_line(&line, &lineTag);

			}
		}
	}

	sprintf(uvPointChar, "max clearance angle is %.4f  \n", maxClearanceAngle);
	UF_UI_write_listing_window(uvPointChar);
	sprintf(uvPointChar, "min clearance angle is %.4f  \n", minClearanceAngle);
	UF_UI_write_listing_window(uvPointChar);

	UF_CURVE_create_point(maxClearanceAnglePoint, &maxClearanceAnglePointTag);
	UF_CURVE_create_point(minClearanceAnglePoint, &minClearanceAnglePointTag);

	/*5.后角可视化（生成对应点的后角生成的直线集合）（已经整合在上面的模块）*/


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
extern int ufusr_ask_unload( void )
{
    return( UF_UNLOAD_IMMEDIATELY );
}

