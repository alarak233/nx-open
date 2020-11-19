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
#include <uf_modl_curves.h>

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
	tag_t scalarTag = NULL_TAG;
	tag_t *pointSetTag;
	double *pointSet;
	int i;
	int j;
	double ctol,atol,stol;
	ctol = 0.000001;
	atol = 0.000001;
	stol = 1.0/80;
	int pointNum;
	char chr[50];
	double basePoint[3] = { 0.0,0.0,0.0 };
	UF_CURVE_line_t line;
	UF_CURVE_create_point(&basePoint[0], &basePointTag);
	line.start_point[0] = 0.0;
	line.start_point[1] = 0.0;
	line.start_point[2] = 0.0;
	line.end_point[0] = 40.0;
	line.end_point[1] = 0.0;
	line.end_point[2] = 30.0;
	UF_CURVE_create_line(&line, &lineTag);
	UF_MODL_ask_curve_points(lineTag,ctol,atol,stol,&pointNum,&pointSet);
	UF_UI_open_listing_window();
	sprintf(chr, "%d\n ", pointNum);
	UF_UI_write_listing_window(chr);
	for (i = 0; i < 3 * pointNum; i += 3)
	{
		UF_UI_write_listing_window("The points are:");
		sprintf(chr, "%.4f ", pointSet[i]);
		UF_UI_write_listing_window(chr);
		sprintf(chr, "%.4f ", pointSet[i+1]);
		UF_UI_write_listing_window(chr);
		sprintf(chr, "%.4f \n", pointSet[i+2]);
		UF_UI_write_listing_window(chr);
	}
	for (i = 0;i<pointNum;i++)
	{
		for (j = 0; j < 3; j++)
			basePoint[j] = pointSet[3 * i + j];
		UF_CURVE_create_point(basePoint,&basePointTag);
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
extern int ufusr_ask_unload( void )
{
    return( UF_UNLOAD_IMMEDIATELY );
}

