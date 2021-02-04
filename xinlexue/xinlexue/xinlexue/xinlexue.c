/*****************************************************************************
**
** xinlexue.c
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
#include <uf_part.h>
#include <direct.h> 
#include <uf_sket.h>
#include <uf_curve.h>
#include <math.h>
#include <uf_modl.h>
#include <uf_csys.h>

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

	int units = UF_PART_METRIC;//公制单位
	int i, j, k;
	double rParameter;

	/*1.获取参数*/

	//底部直径D
	double D = 75.0;
	//基球半径R
	double R = 140.191;
	//高度H
	double H = 30.0;
	//折射率n，计算出的屈光率f
	double n = 1.586;
	double f = (n - 1) / R * 1000;
	//每圈的增加的度数add[10]
	double add[10] = { 200.0,200.0,250.0,250.0,300.0,300.0,350.0,350.0,400.0,400.0 };
	//小球半径r
	double r = 0.9 / 2;
	//小球间距d
	double d = 1.36;
	//最里圈中心距dmin（可计算）
	double dmin = 4 * d;
	//微结构深度h
	double h = 0.0;
	//环深度
	double ch = 0.00051;
	//环宽度
	double cd = 0.065/2.0;

	//获取对话框（一次只能获取12个，分两次）

	//辅助变量
	char allSize[6][16] = { "底部直径D","大片半径R","高度H","折射率n","微结构半径r","微结构间距d" };
	char allAdd[10][16] = { "add1", "add2", "add3", "add4", "add5", "add6", "add7", "add8", "add9", "add10" };
	int *unused = 0;
	double data[6] = { 75.0,140.191,30.0,1.586,0.45,1.36 };
	double dataAdd[10] = { 200.0,200.0,250.0,250.0,300.0,300.0,350.0,350.0,400.0,400.0 };
	uc1609("请输入尺寸", allSize, 6, data, unused);
	D = data[0];
	R = data[1];
	H = data[2];
	n = data[3];
	r = data[4];
	d = data[5];
	uc1609("请输入add", allAdd, 10, dataAdd, unused);
	for (i = 0; i < 10; i++)
	{
		add[i] = dataAdd[i];
	}
	f = (n - 1) / R * 1000;

	//	测试对话框
	/*
	char charData[10][16];
	UF_UI_open_listing_window();
	for ( i = 0; i < 6; i++)
	{
		sprintf(charData[i], "%f", data[i]);
		UF_UI_write_listing_window(charData[i]);
		UF_UI_write_listing_window("\n");
	}
	for (i = 0; i < 10; i++)
	{
		sprintf(charData[i], "%f", dataAdd[i]);
		UF_UI_write_listing_window(charData[i]);
		UF_UI_write_listing_window("\n");
	}
	*/

	/*2.创建新部件*/

	//文件路径
	char dirName[100];
	sprintf(dirName, "C:\\Users\\72707\\Desktop\\R%.0f\\", R);
	_mkdir(dirName);
	char partName[100];
	strcpy(partName, dirName);
	char partChange[20];
	sprintf(partChange, "R%.0f", R);
	strcat(partName, partChange);
	tag_t partTag = NULL_TAG;
	UF_PART_new(partName, units, &partTag);

	/*3.建毛坯*/

	/*用草图旋转由于用到反三角函数不精确，直接用圆柱减球建模

	//横截面草图
	char sectionSketch[20] = "横截面";
	double secSketMatrix[9] = { 1.0,0.0,0.0,0.0,0.0,1.0,0.0,0.0,0.0 };
	tag_t secSketTag = NULL_TAG;
	//辅助变量
	//tag_t object[2];
	//int reference[2];
	//草图初始化
	//UF_SKET_initialize_sketch(sectionSketch, &secSketTag);
	//UF_SKET_create_sketch(sectionSketch, 2, secSketMatrix, object, reference, 1, &secSketTag);

	//画直线及圆弧
	double Rcenter[3] = { 0.0,0.0,0.0 };
	double vector[3] = { 0.0,1.0,0.0 };
	Rcenter[2] = H + sqrt(pow(R, 2) - pow(D / 2, 2));
	tag_t lineTag[2];
	tag_t startPtTag = NULL_TAG;
	tag_t RcenterPtTag = NULL_TAG;
	tag_t arcTag = NULL_TAG;
	UF_CURVE_line_t line[2];

	line[0].start_point[0] = 0.0;
	line[0].start_point[1] = 0.0;
	line[0].start_point[2] = 0.0;
	line[0].end_point[0] = D / 2;
	line[0].end_point[1] = 0.0;
	line[0].end_point[2] = 0.0;
	line[1].start_point[0] = D / 2;
	line[1].start_point[1] = 0.0;
	line[1].start_point[2] = 0.0;
	line[1].end_point[0] = D / 2;
	line[1].end_point[1] = 0.0;
	line[1].end_point[2] = H;

	UF_CURVE_create_point(Rcenter, &RcenterPtTag);
	UF_CURVE_create_point(line[1].end_point, &startPtTag);
	UF_CURVE_create_line(&line[0], &lineTag[0]);
	UF_CURVE_create_line(&line[1], &lineTag[1]);

	//好像要分配内存才能这么写，先不管
	//UF_CURVE_limit_p_t limit[2];
	//limit[0]->limit_type = 0;
	//limit[0]->value = 0;
	//limit[1]->limit_type = UF_CURVE_limit_value;
	//limit[1]->value = asin(D / (2 * R)) / PI * 180;


	UF_CURVE_limit_t limit[2];
	UF_CURVE_limit_p_t pLimit[2] = { &limit[0], &limit[1] };
	limit[0].limit_type = 0;
	limit[0].value = 0;//按值
	limit[1].limit_type = UF_CURVE_limit_value;
	double angle = asin(D / (2*R)) / PI * 180;
	limit[1].value = angle;//按值
	//limit[1].limit_type = UF_CURVE_limit_to_entity;
	//limit[1].limiting_obj = RcenterPtTag;

	//创建XZ基准平面,用来放圆弧,创建普通平面好像没用
	double PlanePoint[3] = { 0.0, 0.0, 0.0 };
	double Direction1[3] = { 0.0, 1.0, 0.0 };
	tag_t DplaneTag = NULL_TAG;
	UF_MODL_create_fixed_dplane(PlanePoint, Direction1, &DplaneTag);

	UF_CURVE_create_arc_point_center(RcenterPtTag, startPtTag, pLimit, DplaneTag, FALSE, &arcTag);
	//	UF_CURVE_create_arc_center_radius(RcenterPtTag, R, startPtTag, pLimit, DplaneTag, false, &arcTag);

	//添加至草图
	//UF_SKET_add_objects(secSketTag, 2, lineTag);

	*/

	//建立圆柱
	double center[3] = { 0.0,0.0,0.0 };
	char height[20];
	sprintf(height, "%.10f", H+1);
	char diameter[20];
	sprintf(diameter, "%.10f", D);
	double direction[3] = { 0.0,0.0,1.0 };
	tag_t cylinderTag = NULL_TAG;
	UF_MODL_create_cylinder(0, NULL_TAG, center, height, diameter, direction, &cylinderTag);
	tag_t cylinderBodyTag = NULL_TAG;
	UF_MODL_ask_feat_body(cylinderTag, &cylinderBodyTag);

	//减球
	double Rcenter[3] = { 0.0,0.0,0.0 };
	Rcenter[2] = H + sqrt(pow(R, 2) - pow(D / 2, 2));
	char Rdiameter[20];
	sprintf(Rdiameter, "%.10f", R * 2);
	tag_t sphereTag = NULL_TAG;
	UF_MODL_create_sphere(2, cylinderBodyTag, Rcenter, Rdiameter, &sphereTag);

	/*4.微结构*/
	//基点坐标
	double points[10][12][3];
	tag_t pointsTag[10][12];
	//double testpoint[3];
	//tag_t testPtTag;
	double angel = 60.0 / 180.0*PI;
	for (i = 0; i < 8; i++)
	{
		for (j = 0; j < i + 4; j++)
		{
			points[i][j][0] = d * (4.0 + i - j / 2.0);
			points[i][j][1] = d * j*sqrt(3.0) / 2.0;
			points[i][j][2] = Rcenter[2] - sqrt(pow(R, 2) - pow(points[i][j][0], 2) - pow(points[i][j][1], 2));
			UF_CURVE_create_point(points[i][j], &pointsTag[i][j]);
			//testpoint[0] = points[i][j][0] * cos(angel) - points[i][j][1] * sin(angel);
			//testpoint[1] = points[i][j][0] * sin(angel) + points[i][j][1] * cos(angel);
			//testpoint[2] = points[i][j][2];
			//UF_CURVE_create_point(testpoint, &testPtTag);
		}
	}
	for (j = 1; j < 12; j++)
	{
		points[8][j][0] = d * (4.0 + 8.0 - j / 2.0);
		points[8][j][1] = d * j*sqrt(3.0) / 2.0;
		points[8][j][2] = Rcenter[2] - sqrt(pow(R, 2) - pow(points[8][j][0], 2) - pow(points[8][j][1], 2));
		UF_CURVE_create_point(points[8][j], &pointsTag[8][j]);
		//testpoint[0] = points[8][j][0] * cos(angel) - points[8][j][1] * sin(angel);
		//testpoint[1] = points[8][j][0] * sin(angel) + points[8][j][1] * cos(angel);
		//testpoint[2] = points[8][j][2];
		//UF_CURVE_create_point(testpoint, &testPtTag);
	}
	for (j = 3; j < 11; j++)
	{
		points[9][j][0] = d * (4.0 + 9.0 - j / 2.0);
		points[9][j][1] = d * j*sqrt(3.0) / 2.0;
		points[9][j][2] = Rcenter[2] - sqrt(pow(R, 2) - pow(points[9][j][0], 2) - pow(points[9][j][1], 2));
		UF_CURVE_create_point(points[9][j], &pointsTag[9][j]);
		//testpoint[0] = points[9][j][0] * cos(angel) - points[9][j][1] * sin(angel);
		//testpoint[1] = points[9][j][0] * sin(angel) + points[9][j][1] * cos(angel);
		//testpoint[2] = points[9][j][2];
		//UF_CURVE_create_point(testpoint, &testPtTag);
	}

	//计算小球半径
	double Ro[10];
	double addf[10];
	for (i = 0; i < 10; i++)
	{
		addf[i] = f + add[i] / 100;
		Ro[i] = (n - 1) / addf[i] * 1000;
	}

	//	测试对话框
	/*
	char charData[10][16];
	UF_UI_open_listing_window();
	sprintf(charData[0], "%f", f);
	UF_UI_write_listing_window(charData[0]);
	UF_UI_write_listing_window("\n");
	for ( i = 0; i < 10; i++)
	{
		sprintf(charData[i], "%f", Ro[i]);
		UF_UI_write_listing_window(charData[i]);
		UF_UI_write_listing_window("\n");
		sprintf(charData[i], "%f", addf[i]);
		UF_UI_write_listing_window(charData[i]);
		UF_UI_write_listing_window("\n");
	}
	*/

	//计算圆心位置
	double ro[10][12][3];
	//辅助变量
	rParameter = 0.0;
	double r2 = pow(r, 2);
	double Rd = sqrt(pow(R, 2) - r2);
	tag_t testPtTag = NULL_TAG;
	double angle = PI / 3;
	double testPoint[3];

	//画微结构
	
	for (i = 0; i <8; i++)
	{
		rParameter = Rd - sqrt(pow(Ro[i], 2) - r2);
		for (j = 0; j < i + 4; j++)
		{
			ro[i][j][0] = rParameter / R * points[i][j][0];
			ro[i][j][1] = rParameter / R * points[i][j][1];
			ro[i][j][2] = (1 - rParameter / R) * Rcenter[2] + rParameter / R * points[i][j][2];
			for (k = 0; k < 6; k++)
			{
				angle = k * PI / 3;
				testPoint[0] = ro[i][j][0] * cos(angle) - ro[i][j][1] * sin(angle);
				testPoint[1] = ro[i][j][0] * sin(angle) + ro[i][j][1] * cos(angle);
				testPoint[2] = ro[i][j][2];
				UF_CURVE_create_point(testPoint, &testPtTag);
				sprintf(diameter, "%.10f", 2 * Ro[i]);
				UF_MODL_create_sphere(2, cylinderBodyTag, testPoint, diameter, &sphereTag);
			}
		}
	}

	rParameter = Rd - sqrt(pow(Ro[8], 2) - r2);
	for (j = 0; j < 12; j++)
	{
		ro[8][j][0] = rParameter / R * points[8][j][0];
		ro[8][j][1] = rParameter / R * points[8][j][1];
		ro[8][j][2] = (1 - rParameter / R) * Rcenter[2] + rParameter / R * points[8][j][2];
		for (k = 0; k < 6; k++)
		{
			angle = k * PI / 3;
			testPoint[0] = ro[8][j][0] * cos(angle) - ro[8][j][1] * sin(angle);
			testPoint[1] = ro[8][j][0] * sin(angle) + ro[8][j][1] * cos(angle);
			testPoint[2] = ro[8][j][2];
			UF_CURVE_create_point(testPoint, &testPtTag);
			sprintf(diameter, "%.10f", 2 * Ro[8]);
			UF_MODL_create_sphere(2, cylinderBodyTag, testPoint, diameter, &sphereTag);
		}
	}

	rParameter = Rd - sqrt(pow(Ro[9], 2) - r2);
	for (j = 1; j < 11; j++)
	{
		ro[9][j][0] = rParameter / R * points[9][j][0];
		ro[9][j][1] = rParameter / R * points[9][j][1];
		ro[9][j][2] = (1 - rParameter / R) * Rcenter[2] + rParameter / R * points[9][j][2];
		for (k = 0; k < 6; k++)
		{
			angle = k * PI / 3;
			testPoint[0] = ro[9][j][0] * cos(angle) - ro[9][j][1] * sin(angle);
			testPoint[1] = ro[9][j][0] * sin(angle) + ro[9][j][1] * cos(angle);
			testPoint[2] = ro[9][j][2];
			UF_CURVE_create_point(testPoint, &testPtTag);
			sprintf(diameter, "%.10f", 2 * Ro[9]);
			UF_MODL_create_sphere(2, cylinderBodyTag, testPoint, diameter, &sphereTag);
		}
	}

	/*5.加环会有环缺失，不知道是什么问题，先做环在做球*/
	//圆结构体
	double martix[9] = { 0.0,1.0,0.0,0.0,0.0,1.0,1.0,0.0,0.0 };
	tag_t circleTag = NULL_TAG;
	UF_CURVE_arc_t circle[17];
	tag_t matrixTag = null_tag;
	UF_CSYS_create_matrix(martix, &matrixTag);

	double r0;
	r0 = (2 * pow(R, 2) /*+ pow(ch, 2)*/ + 2 * R*ch - 2 * (R + ch)*sqrt(pow(R, 2) - pow(cd, 2))) / (2 * R + 2 * ch - 2 * sqrt(pow(R, 2) - pow(cd, 2)));
	rParameter = (R + ch - r0) / R;
	tag_t *mirrorTag = NULL_TAG;
	//以前算的公式
	//(2*R^2+h^2+2*R*h-2*(R+h)*(R^2-d^2)^0.5)/(2*R+2*h-2*(R^2-d^2)^0.5)

	//测试对话框
	/*
	char charData[10][16];
	UF_UI_open_listing_window();
	sprintf(charData[0], "%f", circle[0].radius);
	UF_UI_write_listing_window(charData[0]);
	UF_UI_write_listing_window("\n");
	*/
	for (i = 0; i < 17; i++)
	{
		circle[i].matrix_tag = matrixTag;
		circle[i].radius = r0;
		circle[i].start_angle = 1 * DEGRA;
		circle[i].end_angle = 361 * DEGRA;
		circle[i].arc_center[0] = (i / 2.0 + 3.45)*d*rParameter;
		circle[i].arc_center[2] = 0.0;
		circle[i].arc_center[1] = Rcenter[2] - sqrt(pow(R + ch - r0, 2) - pow(circle[i].arc_center[0], 2));
		UF_CURVE_create_arc(&(circle[i]), &(circleTag));
		//创建槽
		UF_MODL_SWEEP_TRIM_object_t mirror;
		char *limit[2] = { "0.0","360.0" };
		char *offset[2] = { "0.0","0.0" };
		double paraOrigin[3] = { 0.0,0.0,0.0 };
		double axisPoints[3] = { 0.0, 0.0, 0.0 };
		double axisDirection[3] = { 0.0,0.0,1.0 };
		int mirrorNum = 0;

		UF_MODL_create_revolution(&circleTag, 1, &mirror, limit, offset, paraOrigin, false, true, axisPoints, axisDirection, UF_NEGATIVE, &mirrorTag, &mirrorNum);
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

