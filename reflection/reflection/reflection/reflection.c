/*****************************************************************************
**
** reflection.c
**
** Description:
**     Contains Unigraphics entry points for the application.
**
*****************************************************************************/

/* Include files */
#include <stdarg.h>
#include <math.h>
#include <stdio.h>
#include <uf.h>
#include <uf_ui_types.h>
#include <uf_ui.h>
#include <uf_modl.h>
#include <uf_curve.h>


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

	//ȫ�ֱ���
	//ѭ������
	int i = 0;
	int j = 0;
	int l = 0;
	//������
	int response = 0;
	
	/*1.ѡȡ����*/
	tag_t *baseSurfaceTag = NULL_TAG;
	tag_t viewTag = NULL_TAG;
	double cursor[3] = { 0.0,0.0,0.0 };
	int faceNum = 0;

	//ѡȡ�����
	UF_UI_select_with_class_dialog("��ѡ��Ŀ����", "Ŀ����", UF_UI_SEL_SCOPE_ANY_IN_ASSEMBLY, init_proc, NULL, &response, &faceNum, &baseSurfaceTag);

	/*2.���뷴��ⳤ�ȼ������ܼ��̶�*/
	double uvMinMax[4];
	char uvNumChar[3][16] = { "u��","v��" ,"����ⳤ��"};
	double uvNum[3] = { 10,10,150 };
	uc1609("����������ܶ�", uvNumChar, 3, uvNum, 0);

	/*3.�������uvֵ���з���*/
	//�������Χ��������
	//double uvMinParameter[2] = { 0.0,0.0 };
	//double uvMaxParameter[2] = { 0.0,0.0 };
	double uRange=0.0;
	double vRange=0.0;
	//int uvminOrder = 0;
	//int uvmaxOrder = 0;
	//��¼ÿ�����uvminmax
	/*
	for (l = 0; l < faceNum; l++)
	{
		//���ӻ�����ѡȡ������Χ
		UF_MODL_ask_face_uv_minmax(baseSurfaceTag[l], uvMinMax);
		if (uvMinParameter[0] > uvMinMax[0])
		{
			uvMinParameter[0] = uvMinMax[0];
		}
		if (uvMinParameter[1] > uvMinMax[2])
		{
			uvMinParameter[1] = uvMinMax[2];
		}
		if (uvMaxParameter[0] < uvMinMax[1])
		{
			uvMaxParameter[0] = uvMinMax[1];
		}
		if (uvMaxParameter[1] < uvMinMax[3])
		{
			uvMaxParameter[1] = uvMinMax[3];
		}
	}
	uRange = uvMaxParameter[0] - uvMinParameter[0];
	vRange = uvMaxParameter[1] - uvMinParameter[1];

	//����
	
		UF_UI_open_listing_window();
		char test[20];
		sprintf(test, "%.3f %.3f\n",uRange,vRange);
		UF_UI_write_listing_window(test);
	*/

	/*4.��ÿ������ȡ�㣬������������Ⲣ���ӻ���ע���޸���ɫ�ȣ�*/
	//��������
	
	double uvParameter[2] = { 0.0,0.0 };
	double ut = 0.0;
	double vt = 0.0;
	//���ϵ�ͷ�ʸ��������
	double uvPoint[2] = { 0.0,0.0 };
	double uFirstDerivative[3] = { 0.0,0.0,0.0 };
	double vFirstDerivative[3] = { 0.0,0.0,0.0 };
	double uSecondDerivative[3] = { 0.0,0.0,0.0 };
	double vSecondDerivative[3] = { 0.0,0.0,0.0 };
	double normalDirection[3] = { 0.0,0.0,0.0 };
	double reflectionDirection[3] = { 0.0,0.0,0.0 };
	double projection[3] = { 0.0,0.0,0.0 };//ͶӰʸ��
	double projectionLen = 0.0;
	double Light[3] = { 0.0,0.0,1.0 };
	double len = 0;
	double curvatureRadius[2] = { 0.0,0.0 };
	tag_t pointTag = NULL_TAG;
	int isPointOnSurface = 0;
	//���ӻ��õ�ֱ��
	UF_CURVE_line_t line;
	tag_t lineTag = NULL_TAG;
	
	for (l = 0; l < faceNum; l++)
	{
		UF_MODL_ask_face_uv_minmax(baseSurfaceTag[l], uvMinMax);
		for ( i = 0; i <= uvNum[0]; i++)
		{
			ut = i / uvNum[0];
			for (j = 0; j <= uvNum[1]; j++)
			{
				vt = j / uvNum[1];
				ut = ut * 0.99 + 0.005;
				vt = vt * 0.99 + 0.005;
				uvParameter[0] = (uvMinMax[1] - uvMinMax[0])*ut + uvMinMax[0];
				uvParameter[1] = (uvMinMax[3] - uvMinMax[2])*vt + uvMinMax[2];
				UF_MODL_ask_face_props(baseSurfaceTag[l], uvParameter, uvPoint, uFirstDerivative, vFirstDerivative, uSecondDerivative, vSecondDerivative, normalDirection, curvatureRadius);
				/*
				if (normalDirection[2]<0)
				{
					normalDirection[0] = -normalDirection[0];
					normalDirection[1] = -normalDirection[1];
					normalDirection[2] = -normalDirection[2];
				}
				*/

				//�жϵ��Ƿ�������
				UF_MODL_ask_point_containment(uvPoint, baseSurfaceTag[l], &isPointOnSurface);

				if (isPointOnSurface == 1)
				{
					projectionLen = (Light[0] * normalDirection[0] + Light[1] * normalDirection[1] + Light[2] * normalDirection[2]) / (pow(normalDirection[0], 2)+ pow(normalDirection[1], 2)+ pow(normalDirection[2], 2));
					projection[0] = projectionLen * normalDirection[0];
					projection[1] = projectionLen * normalDirection[1];
					projection[2] = projectionLen * normalDirection[2];
					reflectionDirection[0] = 2 * projection[0] - Light[0];
					reflectionDirection[1] = 2 * projection[1] - Light[1];
					reflectionDirection[2] = 2 * projection[2] - Light[2];
					len = pow((pow(reflectionDirection[0], 2) + pow(reflectionDirection[1], 2) + pow(reflectionDirection[2], 2)), 0.5);

					line.start_point[0] = uvPoint[0];
					line.start_point[1] = uvPoint[1];
					line.start_point[2] = uvPoint[2];
					line.end_point[0] = uvPoint[0]+reflectionDirection[0]/len*uvNum[2];
					line.end_point[1] = uvPoint[1]+reflectionDirection[1] / len * uvNum[2];
					line.end_point[2] = uvPoint[2]+reflectionDirection[2] / len * uvNum[2];
					UF_CURVE_create_line(&line, &lineTag);
				}
				
			}
		}
	}
	
	UF_free(&lineTag);
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

