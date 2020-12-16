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

/*ѡȡ�ض���������������ʼ��*/
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

	/*ȫ�ֱ���*/

	int response;
	int i = 0;
	int j = 0;
	int k = 0;
	int l = 0;

	/*1.ѡȡĿ����*/

	tag_t *baseSurfaceTag = NULL_TAG;
	tag_t viewTag = NULL_TAG;
	int faceNum = 0;
	UF_UI_select_with_class_dialog("��ѡ��Ŀ����", "Ŀ����", UF_UI_SEL_SCOPE_ANY_IN_ASSEMBLY, init_proc, NULL, &response, &faceNum, &baseSurfaceTag);

	/*2.ѡȡ���ĵ�*/

//	ʹ���˵��ռ�����Ӧ��ʹ�õ㹹����
/*
	UF_UI_chained_points_t *centralPoint_t[1];
	int centralPointsNum = 1;
	UF_UI_select_point_collection("��ѡ�����ĵ�",false,centralPoint_t,&centralPointsNum,&response);
*/

	UF_UI_POINT_base_method_t pointConstrustMethod = UF_UI_POINT_NO_METHOD;
	tag_t centralPointTag;
	double centralPoint[3] = { 0.0,0.0,0.0 };
	UF_UI_point_construct("��ѡ�����ĵ�", &pointConstrustMethod, &centralPointTag, centralPoint, &response);

	//����
	/*
		UF_UI_open_listing_window();
		char pointCoordinates[20];
		sprintf(pointCoordinates, "%.3f %.3f %.3f\n",centralPoint[0],centralPoint[1],centralPoint[2]);
		UF_UI_write_listing_window(pointCoordinates);
	*/

	/*3.ѡȡ��������˳ʱ�룬��ʱ�룩����ķֲ���uv������*/

/*	ֱ�ӵ���˳��ʱ�������ֵ������ѡ����
	char cuttingDirection[2][38] = { "˳ʱ��","��ʱ��" };
	int isClockwise = 5;
	isClockwise = uc1603("��ѡ����������", 0, cuttingDirection, 2);
*/

//	�������ϵĵ�����Ҫ��uv����Ϊ0-1������Ҫ��ѯ��Ĳ������ع����ɼ����棩
	double uvMinMax[4];

	char uvNumChar[3][16] = { "u��","v��" ,"����Բ���뾶" };
	double uvNum[3] = { 51,51,1 };
	uc1609("��ѡ��u��v������", uvNumChar, 3, uvNum, 0);

	/*4.���ɵ�ķ��������������ת��*/

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
	//char uvPointChar[50];
	int isPointOnSurface = 0;

	//���ӻ��õ�ֱ��
	UF_CURVE_line_t line;
	tag_t lineTag = NULL_TAG;

	//���ϵĵ���������������ƽ�淨����������
	double toolDirection[3] = { 0.0,0.0,0.0 };
	double toolDirectionLength = 0;

	//�������ڼӹ����ϵ�ͶӰ
	double toolPlaneProjection[3] = { 0.0,0.0,0.0 };

	//�������ڼӹ��淨�����ϵ�ͶӰ��ͶӰ����
	double normalVectorProjectionLength = 0;
	double normalVectorProjection[3] = { 0.0,0.0,0.0 };

	//����ͬһ�еĵ�ĸ�������
	//UF_CURVE_create_spline_thru_pts�õ��ĵ�ṹ��
	UF_CURVE_pt_slope_crvatr_t splinePoints[2000];
	//��ʼ��
	for (i = 0; i < 2000; i++)
	{
		splinePoints[i].slope_type = UF_CURVE_SLOPE_NONE;
		splinePoints[i].crvatr_type = UF_CURVE_CRVATR_NONE;
	}
	splinePoints[0].slope_type = UF_CURVE_SLOPE_AUTO;
	//һ�������ϵĵ�ĸ���
	int splinePointNum = 0;

	//���߸�������ʶ����
	tag_t splineTag[2000];
	int splineNum = 0;

	for (l = 0; l < faceNum; l++)
	{
		UF_MODL_ask_face_uv_minmax(baseSurfaceTag[l], uvMinMax);
		//splineNum = 0;

		for (i = 0; i <= uvNum[0]; i++)
		{
			splinePointNum = 0;
			ut = i / uvNum[0];
			ut = ut * 0.9999 + 0.00005;

			for (j = 0; j <= uvNum[1]; j++)
			{
				vt = j / uvNum[1];
				vt = vt * 0.9999 + 0.00005;
				uvParameter[0] = (uvMinMax[1] - uvMinMax[0])*ut + uvMinMax[0];
				uvParameter[1] = (uvMinMax[3] - uvMinMax[2])*vt + uvMinMax[2];
				UF_MODL_ask_face_props(baseSurfaceTag[l], uvParameter, uvPoint, uFirstDerivative, vFirstDerivative, uSecondDerivative, vSecondDerivative, normalDirection, curvatureRadius);

				//�жϵ��Ƿ�������
				UF_MODL_ask_point_containment(uvPoint, baseSurfaceTag[l], &isPointOnSurface);

				if (isPointOnSurface == 1)
				{
					//�������ϵĵ���������������ƽ��ķ�����
					toolDirection[0] = uvPoint[1] - centralPoint[1];
					toolDirection[1] = centralPoint[0] - uvPoint[0];

					//���㷨�����ڼӹ�ƽ���ϵ�ͶӰ����
					//�ȼ��㷨������ƽ�淨�����ϵ�ͶӰ(toolDirection[2]=0,�����ֶ��Ż�)
					toolDirectionLength = sqrt(pow(toolDirection[0], 2) + pow(toolDirection[1], 2) + pow(toolDirection[2], 2));
					normalVectorProjectionLength = (toolDirection[0] * normalDirection[0] + toolDirection[1] * normalDirection[1] + toolDirection[2] * normalDirection[2]) / toolDirectionLength;
					normalVectorProjection[0] = toolDirection[0] * normalVectorProjectionLength / toolDirectionLength;
					normalVectorProjection[1] = toolDirection[1] * normalVectorProjectionLength / toolDirectionLength;
					normalVectorProjection[2] = toolDirection[2] * normalVectorProjectionLength / toolDirectionLength;
					//Ȼ������ڼӹ�ƽ���ϵķ�����
					toolPlaneProjection[0] = normalDirection[0] - normalVectorProjection[0];
					toolPlaneProjection[1] = normalDirection[1] - normalVectorProjection[1];
					toolPlaneProjection[2] = normalDirection[2] - normalVectorProjection[2];
					//���ݵ��߰뾶�ı�������С
					toolDirectionLength = sqrt(pow(toolPlaneProjection[0], 2) + pow(toolPlaneProjection[1], 2) + pow(toolPlaneProjection[2], 2));
					if (toolPlaneProjection[2] > 0)
					{
						toolPlaneProjection[0] = toolPlaneProjection[0] * uvNum[2] / toolDirectionLength;
						toolPlaneProjection[1] = toolPlaneProjection[1] * uvNum[2] / toolDirectionLength;
						toolPlaneProjection[2] = toolPlaneProjection[2] * uvNum[2] / toolDirectionLength;
					}
					else
					{
						toolPlaneProjection[0] = -toolPlaneProjection[0] * uvNum[2] / toolDirectionLength;
						toolPlaneProjection[1] = -toolPlaneProjection[1] * uvNum[2] / toolDirectionLength;
						toolPlaneProjection[2] = -toolPlaneProjection[2] * uvNum[2] / toolDirectionLength;
					}

					//sprintf(uvPointChar, "%.4f %.4f %.4f %.4f \n", uvPoint[0], uvPoint[1], uvPoint[2], clearanceAngle);//���������͵�ĺ��
					//UF_CURVE_create_point(uvPoint, &uvPointTag);

					//��ǿ��ӻ�
					line.start_point[0] = uvPoint[0];
					line.start_point[1] = uvPoint[1];
					line.start_point[2] = uvPoint[2];
					line.end_point[0] = uvPoint[0] + toolPlaneProjection[0];
					line.end_point[1] = uvPoint[1] + toolPlaneProjection[1];
					line.end_point[2] = uvPoint[2] + toolPlaneProjection[2];
					UF_CURVE_create_line(&line, &lineTag);

					//��¼������
					splinePoints[splinePointNum].point[0] = line.end_point[0];
					splinePoints[splinePointNum].point[1] = line.end_point[1];
					splinePoints[splinePointNum].point[2] = line.end_point[2];

					//���ʷ���
					//splinePoints[splinePointNum].crvatr[0] = -toolPlaneProjection[0];
					//splinePoints[splinePointNum].crvatr[1] = -toolPlaneProjection[1];
					//splinePoints[splinePointNum].crvatr[2] = -toolPlaneProjection[2];

					//�����
					splinePointNum++;

				}
			}
			if (splinePointNum > 1)
			{
				splinePoints[splinePointNum].slope_type = UF_CURVE_SLOPE_AUTO;
				UF_CURVE_create_spline_thru_pts(3, 0, splinePointNum, splinePoints, NULL, 1, &(splineTag[splineNum]));
				splineNum++;
				splinePoints[splinePointNum].slope_type = UF_CURVE_SLOPE_NONE;
			}
		}
	}

	//����
	/*
	UF_UI_open_listing_window();
	char test[20];
	sprintf(test, "%d", splineNum);
	UF_UI_write_listing_window(test);
	*/

	//�������߼��ϼ���ʼ����Ϊ����������׼��
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
	double value[6] ;
	int vdegreee = 3;
	int vstatus = 0;
	int bodytype = 1;
	double tolerance[3] = { 0.000011,0.0001,0.1 };
	tag_t neighborSurface[2] = { NULL_TAG,NULL_TAG };
	int constraint[2] = { 0,0 };
	tag_t finalSurface = NULL_TAG;

	//������������
	UF_MODL_create_thru_curves(&splineList, &spineList, &patch, &alignment, value, &vdegreee, &vstatus, &bodytype, UF_NULLSIGN, tolerance, neighborSurface, constraint, &finalSurface);
	UF_MODL_free_string_list(&splineList);
	//UF_MODL_free_string_list(&spineList);

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

