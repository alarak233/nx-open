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
	/*1.选择反射镜形状（圆形，方形）*/

	/*2.输入所需的参数：
		1）.离轴距离d
		2）.抛物线关键参数y=1/2p*x^2
		3）.反射镜高度h或反射镜中心高度ho
		4）.反射镜底面大小（圆形r，方形a，b，其他类型自改）*/

	/*3.判断工装类型（吸盘式，螺丝固定）*/

	/*4.选择文件夹，创建新部件工件*/

	/*5.在工件中建模*/

	/*6.创建新部件，工装*/

	/*7.在工装中建模*/

	/*8.创建新部件粗车工件*/

	/*9.在粗车工装中建模*/

	/*10.新建图纸并出图*/

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

