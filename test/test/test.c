/*****************************************************************************
**
** test.c
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
/*  Unigraphics Startup
**      This entry point activates the application at Unigraphics startup */
extern DllExport void ufsta( char *param, int *returnCode, int rlen )
{
    /* Initialize the API environment */
    if( UF_CALL(UF_initialize()) ) 
    {
        /* Failed to initialize */
        return;
    }
    
    /* TODO: Add your application code here */
	uc1601("测试中文", 1);
	logical Is_open;
	UF_UI_is_listing_window_open(&Is_open);
	if (!Is_open) UF_UI_open_listing_window();
	UF_UI_write_listing_window("第一次写入信息窗口的信息\n");
	uc1601("将要关闭信息窗口", 1);
	UF_UI_close_listing_window();
	uc1601("将要显示信息窗口", 1);
	UF_UI_open_listing_window();
	UF_UI_write_listing_window("第二次写入信息窗口的信息\n");
	uc1601("将要退出信息窗口", 1);
	UF_UI_exit_listing_window();
	uc1601("将要显示信息窗口", 1);
	UF_UI_open_listing_window();
	UF_UI_write_listing_window("第三次写入信息窗口的信息\n");
	UF_UI_save_listing_window("D:\\Info.txt");
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

