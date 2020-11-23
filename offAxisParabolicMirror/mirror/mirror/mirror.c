/*****************************************************************************
**
** mirror.c
**
** Description:
**     Contains Unigraphics entry points for the application.
**
*****************************************************************************/

/* Include files */
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <uf.h>
#include <uf_ui_types.h>
#include <uf_ui.h>

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
	/*1.ѡ���侵��״��Բ�Σ����Σ�*/
	
	int isCircle = 5;	//uc1603����ֵ5-18
	char shape[2][38] = {"Բ��","����"};
	isCircle = uc1603("��ѡ�����ﾵ��״",0,shape,2);
	if (isCircle == 1 || isCircle == 2)
		UF_CALL(UF_terminate());

//	���ԶԻ���
/*	
	char charIsCircle[30];
	UF_UI_open_listing_window();
	if (isCircle == 5)
	{
		UF_UI_write_listing_window("Բ��\n");
	}
	else if (isCircle == 6)
	{
		UF_UI_write_listing_window("����\n");
	}
	sprintf(charIsCircle, "%d", isCircle);
	UF_UI_write_listing_window(charIsCircle);
*/
	/*2.��������Ĳ�����
		1��.�������d
		2��.�����߹ؼ�����y=1/2p*x^2
		3��.���侵�߶�h���侵���ĸ߶�ho
		4��.���侵�����С��Բ��r������a��b�����������Ըģ�*/

	double d,p,h,r,a,b;
	char circleSize[4][16] = { "�������","����(p)y=1/px^2","�ܸ߶�","�ײ��뾶" };
	char squareSize[5][16] = { "�������","����(p)y=1/px^2","�ܸ߶�","����������","������ֱ����" };
	double data[5] = { 0,0,0,0,0 };
	int *unused = 0;
	if (isCircle == 5)
	{
		uc1609("������ߴ�", circleSize, 4, data, unused);
		d = data[0];
		p = data[1] / 2;
		h = data[2];
		r = data[3];
	}
	else
	{
		uc1609("������ߴ�", squareSize, 5, data, unused);
		d = data[0];
		p = data[1] / 2;
		h = data[2];
		a = data[3];
		b = data[4];
	}

//���ԶԻ���
	
	char charData[5][16];
	int i = 1;
	UF_UI_open_listing_window();
	for ( i = 0; i < 5; i++)
	{
		sprintf(charData[i], "%f", data[i]);
		UF_UI_write_listing_window(charData[i]);
		UF_UI_write_listing_window("\n");
	}
	

	/*3.�жϹ�װ���ͣ�����ʽ����˿�̶���*/

	/*4.ѡ���ļ��У������²�������*/

	/*5.�ڹ����н�ģ*/

	/*6.�����²�������װ*/

	/*7.�ڹ�װ�н�ģ*/

	/*8.�����²����ֳ�����*/

	/*9.�ڴֳ���װ�н�ģ*/

	/*10.�½�ͼֽ����ͼ*/

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
