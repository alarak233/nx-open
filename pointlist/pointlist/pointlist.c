/*****************************************************************************
**
** pointlist.c
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
#include <uf_point.h>
#include <uf_curve.h>
#include <uf_so.h>

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
	tag_t lineTag = NULL_TAG;
	tag_t pointTag = NULL_TAG;
	tag_t basePointTag = NULL_TAG;
	tag_t scalar = NULL_TAG;
	double basePoint[3] = { 0.0,0.0,0.0 };
	UF_CURVE_line_t line;
	UF_SO_set_double_of_scalar(scalar, 1);
	UF_CURVE_create_point(basePoint, &basePointTag);
	line.start_point[0] = 0.0;
	line.start_point[1] = 0.0;
	line.start_point[2] = 0.0;
	line.end_point[0] = 20.0;
	line.end_point[1] = 0.0;
	line.end_point[2] = 5.0;
	UF_CURVE_create_line(&line, &lineTag);
	UF_POINT_create_along_curve(lineTag, basePointTag, scalar, UF_SO_point_along_curve_distance, FALSE, &pointTag);
	uc1601("", 1);

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

