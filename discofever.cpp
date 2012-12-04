//////////////////////////////////////////////////////////////////////////////////////        
// discofever.cpp
//
// Written by Dan, Matt, and Tianyi
//
// Creats a dirty dancefloor of wild imagination and endless possibility
//////////////////////////////////////////////////////////////////////////////////////

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <png.h>
#include <GL/glut.h>

#define PI 3.14159265

using namespace std;

// Globals.
static bool fogUp = true;
static bool isAnimate = 0;

static float spotAngle = 25.0; // Spotlight cone half-angle.
static float xMove = 0.0, zMove = 0.0; // Movement components.
static float xRot = 0.0, yRot = 180.0; // Rotation components.
static float spotExponent = 10.0; // Spotlight exponent = attenuation.

static int redUp = 1;
static int greenUp = 1;
static int blueUp = 0;
static float redIntensity = 0.3;
static float greenIntensity = 0.65;
static float blueIntensity = 1.0; 
static float intensityStep = 0.05; // Spotlight exponent = attenuation.

static int aniTime = 20; // Spotlight exponent = attenuation.
static float discoBallRotation = 0.0; // Spotlight exponent = attenuation.
static float spotLightParameter = 0.0; // Spotlight exponent = attenuation.
static float upperBound = 1.0; // Spotlight exponent = attenuation.
static float lowerBound = 0.3; // Spotlight exponent = attenuation.
static float global_fog_density = .09;

static long font = (long)GLUT_BITMAP_8_BY_13; // Font selection.
static char theStringBuffer[10]; // String buffer.

//Game Booleans
static int game_guess_time = 750;
static int game_current_ans = -1;
static int game_user_ans = -1;
static int game_num_correct = 0;
static int game_num_total = 10;
static int game_num_round = 0;
static int game_level = 1;
static bool game_on = false;

static float colors[] = {
0.800, 0.800, 0.800,
1.000, 0.800, 0.000,
0.000, 1.000, 0.000,
0.667, 0.000, 0.471,
0.419, 0.792, 0.886,
0.667, 0.949, 0.000,
1.000, 0.000, 0.000 
};

GLubyte *textureImage;

bool loadPngImage(char *name, int &outWidth, int &outHeight, bool &outHasAlpha, GLubyte **outData) {
    png_structp png_ptr;
    png_infop info_ptr;
    unsigned int sig_read = 0;
    int color_type, interlace_type;
    FILE *fp;

    if ((fp = fopen(name, "rb")) == NULL)
        return false;

    /* Create and initialize the png_struct
     * with the desired error handler
     * functions.  If you want to use the
     * default stderr and longjump method,
     * you can supply NULL for the last
     * three parameters.  We also supply the
     * the compiler header file version, so
     * that we know if the application
     * was compiled with a compatible version
     * of the library.  REQUIRED
     */
    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
            NULL, NULL, NULL);

    if (png_ptr == NULL) {
        fclose(fp);
        return false;
    }

    /* Allocate/initialize the memory
     * for image information.  REQUIRED. */
    info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == NULL) {
        fclose(fp);
        png_destroy_read_struct(&png_ptr, png_infopp_NULL, png_infopp_NULL);
        return false;
    }

    /* Set error handling if you are
     * using the setjmp/longjmp method
     * (this is the normal method of
     * doing things with libpng).
     * REQUIRED unless you  set up
     * your own error handlers in
     * the png_create_read_struct()
     * earlier.
     */
    if (setjmp(png_jmpbuf(png_ptr))) {
        /* Free all of the memory associated
         * with the png_ptr and info_ptr */
        png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);
        fclose(fp);
        /* If we get here, we had a
         * problem reading the file */
        return false;
    }

    /* Set up the output control if
     * you are using standard C streams */
    png_init_io(png_ptr, fp);

    /* If we have already
     * read some of the signature */
    png_set_sig_bytes(png_ptr, sig_read);

    /*
     * If you have enough memory to read
     * in the entire image at once, and
     * you need to specify only
     * transforms that can be controlled
     * with one of the PNG_TRANSFORM_*
     * bits (this presently excludes
     * dithering, filling, setting
     * background, and doing gamma
     * adjustment), then you can read the
     * entire image (including pixels)
     * into the info structure with this
     * call
     *
     * PNG_TRANSFORM_STRIP_16 |
     * PNG_TRANSFORM_PACKING  forces 8 bit
     * PNG_TRANSFORM_EXPAND forces to
     *  expand a palette into RGB
     */
    png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_STRIP_16 | PNG_TRANSFORM_PACKING | PNG_TRANSFORM_EXPAND, png_voidp_NULL);

    outWidth = info_ptr->width;
    outHeight = info_ptr->height;
    switch (info_ptr->color_type) {
        case PNG_COLOR_TYPE_RGBA:
            outHasAlpha = true;
            break;
        case PNG_COLOR_TYPE_RGB:
            outHasAlpha = false;
            break;
        default:
            std::cout << "Color type " << info_ptr->color_type << " not supported" << std::endl;
            png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
            fclose(fp);
            return false;
    }
    unsigned int row_bytes = png_get_rowbytes(png_ptr, info_ptr);
    *outData = (unsigned char*) malloc(row_bytes * outHeight);

    png_bytepp row_pointers = png_get_rows(png_ptr, info_ptr);

    for (int i = 0; i < outHeight; i++) {
        // note that png is ordered top to
        // bottom, but OpenGL expect it bottom to top
        // so the order or swapped
        memcpy(*outData+(row_bytes * (outHeight-1-i)), row_pointers[i], row_bytes);
    }

    /* Clean up after the read,
     * and free any memory allocated */
    png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);

    /* Close the file */
    fclose(fp);

    /* That's it */
    return true;
}

void loadTextureFromPng(char* filename)
{
	int width, height;
	bool hasAlpha;
	bool success = loadPngImage(filename, width, height, hasAlpha, &textureImage);
	if (!success) {
	    std::cout << "Unable to load png file" << std::endl;
	    return;
	}

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(GL_TEXTURE_2D, 0, hasAlpha ? 4 : 3, width,
                height, 0, hasAlpha ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE,
                textureImage);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}

// Routine to draw a bitmap character string.
void writeBitmapString(void *font, char *string)
{  
   char *c;

   for (c = string; *c != '\0'; c++) glutBitmapCharacter(font, *c);
}

void resetGame()
{
	game_current_ans = -1;
	game_user_ans = -1;
	game_num_correct = 0;
	game_num_round = 0;
	game_on = false;
}

void rungame(int value)
{
	//Starts first round of game by generating random answer and clearing user answer
	if(game_current_ans == -1 and game_on)
	{
		game_current_ans = rand()%3;
		game_user_ans = -1;
	}
	else if(game_on)
	{		
		//When this called, the guessing period is up so we see if the user 
		//guessed it correctly
		if(game_current_ans == game_user_ans)
		{
			game_num_correct +=1;
			cout<<"Correct"<<endl;
		}
		else
		{
			cout<<"FUCKING WRONG, KID"<<endl;
		}
		//Randomly picks a new number to guess that is different from
		//the last random number
		int new_ans = rand()%3;
		while(new_ans == game_current_ans)
			new_ans = rand()%3;
		game_current_ans = new_ans;
		
		game_user_ans = -1;
		cout<<(game_current_ans+1)<<endl;
		game_num_round += 1;
		
		//Resets the game when it is over
		if(game_num_round >= game_num_total)
		{
                        double percentCorrect = 100.0*(double)game_num_correct/(double)game_num_total;
			cout<<"Percent Correct: "<< percentCorrect <<endl;
                        if (percentCorrect >= 80 && game_guess_time > 150)
                        {
                            game_level += 1;
                            char pngFile[9];
                            sprintf(pngFile,"lvl%d.png",game_level);
                            loadTextureFromPng(pngFile);
	        	}
			resetGame();
		}
	}
	else
		true;
	
	glutTimerFunc(max(game_guess_time-(game_level-1)*100,150),rungame,1);
	glutPostRedisplay();
}

void animate(int value)
{
	if(isAnimate)
	{
	    if(fogUp)
	        global_fog_density +=.001;
	    else
	    	global_fog_density -=.001;
	    if(global_fog_density >=.15)
	    	fogUp = false;
	    if(global_fog_density <=.09)
	    	fogUp = true;
	    
	    discoBallRotation += .1;
	    spotLightParameter += .05;
	    if (discoBallRotation >= 361)
		discoBallRotation = 0;
	    if (spotLightParameter >= 361)
		spotLightParameter = 0;

	     if (redIntensity <= lowerBound) { 
		 redUp = 1;
		 redIntensity = lowerBound;
	     }
	     if (redIntensity >= upperBound) {
		 redUp = 0;
		 redIntensity = upperBound;
	     }
	     if (redUp)
		 redIntensity += intensityStep; 
	     else 
		 redIntensity -= intensityStep; 

	     if (greenIntensity <= lowerBound) { 
		 greenUp = 1;
		 greenIntensity = lowerBound;
	     }
	     if (greenIntensity >= upperBound) {
		 greenUp = 0;
		 greenIntensity = upperBound;
	     }
	     if (greenUp)
		 greenIntensity += intensityStep; 
	     else 
		 greenIntensity -= intensityStep; 

	     if (blueIntensity <= lowerBound) { 
		 blueUp = 1;
		 blueIntensity = lowerBound;
	     }
	     if (blueIntensity >= upperBound) {
		 blueUp = 0;
		 blueIntensity = upperBound;
	     }
	     if (blueUp)
		 blueIntensity += intensityStep; 
	     else 
		 blueIntensity -= intensityStep; 
	}

	glutTimerFunc(aniTime,animate,1);
	glutPostRedisplay();
}

// Routine to convert floating point to char string.
void floatToString(char * destStr, int precision, float val) 
{
   sprintf(destStr,"%f",val);
   destStr[precision] = '\0';
}



// Initialization routine.
void setup(void)
{
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glEnable(GL_DEPTH_TEST); // Enable depth testing.
	glEnable(GL_BLEND); // Enable blending.

        glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        char pngFile[] = "lvl1.png";
        loadTextureFromPng(pngFile);

	// Turn on OpenGL lighting.
	glEnable(GL_LIGHTING);

	// Light property vectors.
	float lightAmb[] = { 0.0, 0.0, 0.0, 1.0 };
	float lightDifAndSpec[] = { 1.0, 1.0, 1.0, 1.0 };
	float globAmb[] = { 0.5, 0.5, 0.5, 1.0 };

	// Light properties.
	glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmb);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDifAndSpec);
	glLightfv(GL_LIGHT0, GL_SPECULAR, lightDifAndSpec);

	// Light properties.
	glLightfv(GL_LIGHT1, GL_AMBIENT, lightAmb);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, lightDifAndSpec);
	glLightfv(GL_LIGHT1, GL_SPECULAR, lightDifAndSpec);

	// Light properties.
	glLightfv(GL_LIGHT2, GL_AMBIENT, lightAmb);
	glLightfv(GL_LIGHT2, GL_DIFFUSE, lightDifAndSpec);
	glLightfv(GL_LIGHT2, GL_SPECULAR, lightDifAndSpec);

	// Light property vectors.
	float lightAmb3[] = { 0.0, 0.0, 0.0, 1.0 };
	float lightDifAndSpec3[] = { 0.2, 0.2, 0.2, 1.0 };
	float lightPos3[] = {0.0, 4.0, 0.0, 0.0 }; // Overhead directional light source (e.g., sun).

	// Light properties.
	glLightfv(GL_LIGHT3, GL_AMBIENT, lightAmb3);
	glLightfv(GL_LIGHT3, GL_DIFFUSE, lightDifAndSpec3);
	glLightfv(GL_LIGHT3, GL_SPECULAR, lightDifAndSpec3);
	glLightfv(GL_LIGHT3, GL_POSITION, lightPos3);

	float spotDirection3[] = {0.0, -1.0, 0.0, 1.0}; // Spotlight direction.
	glLightfv(GL_LIGHT3, GL_SPOT_DIRECTION, spotDirection3);    
	glLightf(GL_LIGHT3, GL_SPOT_CUTOFF, 1.25);

	// Light property vectors.
	float lightAmb4[] = { 0.0, 0.0, 0.0, 1.0 };
	float lightDifAndSpec4[] = { .5, .5, .5, 1.0 };
	float lightPos4[] = {0.0, -1.0, 0.0, 0.0 }; // Overhead directional light source (e.g., sun).

	// Light properties.
	glLightfv(GL_LIGHT4, GL_AMBIENT, lightAmb4);
	glLightfv(GL_LIGHT4, GL_DIFFUSE, lightDifAndSpec4);
	glLightfv(GL_LIGHT4, GL_SPECULAR, lightDifAndSpec4);
	glLightfv(GL_LIGHT4, GL_POSITION, lightPos4);

	glEnable(GL_LIGHT0); // Enable particular light source.
	glEnable(GL_LIGHT1); // Enable particular light source.
	glEnable(GL_LIGHT2); // Enable particular light source.
	glEnable(GL_LIGHT3); // Enable particular light source.
	glEnable(GL_LIGHT4); // Enable particular light source.
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globAmb); // Global ambient light.
	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE); // Enable local viewpoint.

	// Material property vectors.
	float matSpec[] = { 1.0, 1.0, 1.0, 1.0 };
	float matShine[] = { 500.0 };

	// Material properties shared by all the spheres.
	glMaterialfv(GL_FRONT, GL_SPECULAR, matSpec);
	glMaterialfv(GL_FRONT, GL_SHININESS, matShine);

	// Cull back faces.
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	//Dat Fog Baby
	glEnable (GL_DEPTH_TEST); //enable the depth testing
	glEnable (GL_FOG); //enable the fog
	glFogi (GL_FOG_MODE, GL_EXP2); //set the fog mode to GL_EXP2
	glFogf (GL_FOG_DENSITY, global_fog_density); //set the density to the value above
	glHint (GL_FOG_HINT, GL_NICEST); // set the fog to look the nicest, may slow down on older cards

	// Enable color material mode:
	// The ambient and diffuse color of the front faces will track the color set by glColor().
	glEnable(GL_COLOR_MATERIAL); 
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
}

void drawDanceFloor()
{
	glPopMatrix();
	// Draw 10 x 10 array of multi-colored spheres.
	int i,j;
	for (i = 0; i < 50; i++)
	{
		for (j = 0; j < 50; j++)
		{
			glPushMatrix();
			glTranslatef(-20.0+i, 0.0, -20.0+j);

			// Ambient and diffuse colors of the spheres specified to alternate.
			if ((i+j)%3 == 0) { 
				glColor4f(redIntensity, 0.0, 0.0, 1.0);
			}
			else if ((i+j)%3 == 1) {
				glColor4f(0.0, greenIntensity, 0.0, 1.0);
			}
			else {
				glColor4f(0.0, 0.0, blueIntensity, 1.0);
			}
			//int randomColor = 3*(rand()%7);
			//glColor4f(colors[randomColor], colors[randomColor+1], colors[randomColor+2], 1.0);

			glShadeModel(GL_SMOOTH);

			glutSolidCube(1.0);
			glPopMatrix(); 
		}
	}
	glPushMatrix();
}

void drawDiscoStick()
{
	glPushMatrix();
	glColor3f(0.0, 0.0, 0.0);
	glTranslatef(0.0,2.5,-4.0);
	for(int  i = 0; i <20; i++)
	{
		glTranslatef(0.0,.3,0.0);
		glutSolidCube(.3);
	}
	glPopMatrix();
}

void draw3GameBalls()
{
	glShadeModel(GL_SMOOTH);
	glPushMatrix();
	if(game_current_ans == 2)
		glColor3f(1.0, 1.0, 1.0); 
	else
		glColor3f(0.0, 0.0, 0.0);
	glTranslatef(1.25,1.,-2.0);
	glutSolidSphere(0.5,20,20);
	if(game_current_ans == 1)
		glColor3f(1.0, 1.0, 1.0); 
	else
		glColor3f(0.0, 0.0, 0.0);
	glTranslatef(-1.25,0.,0.0);
	glutSolidSphere(0.5,20,20);
	if(game_current_ans == 0)
		glColor3f(1.0, 1.0, 1.0); 
	else
		glColor3f(0.0, 0.0, 0.0);
	glTranslatef(-1.25,0.,0.0);
	glutSolidSphere(0.5,20,20);
	
	glPopMatrix();
}

void drawLevelScreen()
{
        glPushMatrix();
	glTranslatef(1.0, 2.5, -3.6);
	glRotatef(yRot, 0,1,0);
	glRotatef(xRot, 1,0,0);

        //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(-2.0, -1.0, 0.0);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(-2.0, 1.0, 0.0);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(0.0, 1.0, 0.0);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(0.0, -1.0, 0.0);

	glEnd();
	glDisable(GL_TEXTURE_2D);
        glPopMatrix();
}

void writeMessageToScreen()
{
	glDisable(GL_LIGHTING);
	glColor3f(1.0, 1.0, 1.0);
	floatToString(theStringBuffer, 4, spotExponent);
	glRasterPos3f(-1.0, 1.0, -2.0);
	char theUpperMessage[] = "Dance Untill the World Ends, Bitch!";
	writeBitmapString((void*)font, theUpperMessage);  
	//writeBitmapString((void*)font, theStringBuffer);
	glEnable(GL_LIGHTING);
}

// Drawing routine.
void drawScene()
{
	
	float lightPos[] = { 0.0, 3.0, 0.0, 1.0 }; // Spotlight position.
	float spotDirection[] = {0.0, -1.0, 0.0}; // Spotlight direction.   

	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glLoadIdentity();

	float matAmbAndDif1[] = {0.9, 0.0, 0.0, .5};
	float matAmbAndDif2[] = {0.9, 0.9, 0.9, 1.0};
	float matSpec[] = { 1.0, 1.0, 1.0, 1.0 };
	float matShine[] = { 40.0 };

	writeMessageToScreen();

	gluLookAt (xMove/2.5, 3.0, 5.0+zMove/2.5, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0);

	glPushMatrix();
	zMove = 2*(2 + cos(2 * spotLightParameter))*cos(3 * spotLightParameter);
	xMove = 2*(2 + cos(2 * spotLightParameter))*sin(3 * spotLightParameter);
	glTranslatef(xMove, 0.0, zMove); // Move the spotlight.

	// Draw the spotlight cone in wireframe after disabling lighting
	glPushMatrix();
	glDisable(GL_LIGHTING);
	glRotatef(-90.0, 1.0, 0.0, 0.0);
	glColor3f(1.0, 1.0, 1.0);
	//glutWireCone(3.0 * tan( spotAngle/180.0 * PI ), 3.0, 20, 20);
	glEnable(GL_LIGHTING);
	glPopMatrix();

	// Spotlight properties including position.
	glLightfv(GL_LIGHT0, GL_POSITION, lightPos);  
	glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, spotAngle);
	glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, spotDirection);    
	glLightf(GL_LIGHT0, GL_SPOT_EXPONENT, spotExponent);

	glPopMatrix();

	glPushMatrix();

	zMove = 2*(2 + cos(2 * spotLightParameter+90))*cos(3 * spotLightParameter+90);
	xMove = 2*(2 + cos(2 * spotLightParameter-90))*sin(3 * spotLightParameter-90);
	glTranslatef(xMove, 0.0, zMove); // Move the spotlight.

	// Spotlight properties including position.
	glLightfv(GL_LIGHT2, GL_POSITION, lightPos);  
	glLightf(GL_LIGHT2, GL_SPOT_CUTOFF, spotAngle);
	glLightfv(GL_LIGHT2, GL_SPOT_DIRECTION, spotDirection);    
	glLightf(GL_LIGHT2, GL_SPOT_EXPONENT, spotExponent);

	glPopMatrix();

	glPushMatrix();

	zMove = 2*(2 + cos(1 * spotLightParameter+90))*cos(1 * spotLightParameter+90);
	xMove = 2*(2 + cos(1 * spotLightParameter-90))*sin(1 * spotLightParameter-90);
	glTranslatef(xMove, 0.0, zMove); // Move the spotlight.

    // Spotlight properties including position.
	glLightfv(GL_LIGHT1, GL_POSITION, lightPos);  
	glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, spotAngle);
	glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, spotDirection);    
	glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, spotExponent);

	//Draws the raving dance floor
	drawDanceFloor();
	
	//Draws the Discoball
	glColor4f(1.0, 1.0, 1.0, 1.0);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, matAmbAndDif1);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, matSpec);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, matShine);
	glShadeModel(GL_FLAT);
	glTranslatef(0.,3.,-4.0);
	glRotatef(90,1.0,0.0,0.0);
	glRotatef(discoBallRotation*10,0.0,0.0,1.0);
	glutSolidSphere(0.75,20,20);
	glPopMatrix();

	//Draws the Discoball's pole
	drawDiscoStick();
	
	//Draws 3 game balls
	draw3GameBalls();
	
    //Draws Back Wall
    glPushMatrix();
	glColor3f(0.5, 0.35, .05);
	glTranslatef(0.,0.,-35);
	glutSolidCube(50);
	glPopMatrix();

	//Draws the dynamic fog
    float fogColor[4] = {redIntensity, greenIntensity, blueIntensity,1};
	glFogfv (GL_FOG_COLOR, fogColor); //set the fog color to our color chosen above
	glFogf (GL_FOG_DENSITY, global_fog_density); //set the density to the value above

        //Draw the quad
        drawLevelScreen();

	glutSwapBuffers();
}

// OpenGL window reshape routine.
void resize (int w, int h)
{
   glViewport (0, 0, (GLsizei)w, (GLsizei)h);
   glMatrixMode (GL_PROJECTION);
   glLoadIdentity();
   gluPerspective(60.0, (float)w/(float)h, 1.0, 20.0);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
}

// Keyboard input processing routine.
void keyInput(unsigned char key, int x, int y)
{
   switch (key) 
   {
	   case 27:
		   exit(0);
		   break;
	   case 't':
		   if (spotExponent > 0.0) spotExponent -= 0.1;
		   glutPostRedisplay();
		   break;
	   case ' ':
                   game_on = !game_on;
                   game_user_ans = -1;
		   break;
	   case 'a':
		   isAnimate = !isAnimate;
		   glutPostRedisplay();
		   break;
	   case 'T':
		   spotExponent += 0.1;
		   glutPostRedisplay();
		   break;
	   case '1':
		   game_user_ans = 0;
		   break;
	   case '2':
		   game_user_ans = 1;
		   break;
	   case '3':
		   game_user_ans = 2;
		   break; 
	   default:
		   break;
   }
}

// Callback routine for non-ASCII key entry.
void specialKeyInput(int key, int x, int y)
{
   //These don't do anything, but should some day
   if (key == GLUT_KEY_PAGE_DOWN)
   {
      if (spotAngle > 0.0) spotAngle -= 1.0;
   }
   if( key == GLUT_KEY_PAGE_UP)
   {
      if (spotAngle < 90.0) spotAngle += 1.0;
   }
   if (key == GLUT_KEY_UP)
   {
      if (yRot < 360.0) yRot += 10.0;
      cout << "Y Rotation:" << yRot << endl;
   }
   if (key == GLUT_KEY_DOWN)
   {
      if (yRot > 0.0) yRot -= 10.0;
      cout << "Y Rotation:" << yRot << endl;
   }
   if (key == GLUT_KEY_LEFT)
   {
      if (xRot < 360.0) xRot += 10.0;
      cout << "X Rotation:" << xRot << endl;
   }
   if (key == GLUT_KEY_RIGHT)
   {
      if (xRot > 0.0) xRot -= 10.0;
      cout << "X Rotation:" << xRot << endl;
   }
   glutPostRedisplay();
}

void rightMenu(int id)
{
   if (id==0) exit(0);
   if (id==1) isAnimate = !isAnimate;
   if (id==2) game_on = !game_on;
}

// Function to create menu.
void makeMenu(void)
{
/*
   int sub_menu;
   sub_menu = glutCreateMenu(grid_menu);
   glutAddMenuEntry("On", 3);
   glutAddMenuEntry("Off",4);
   glutAddSubMenu("Grid", sub_menu);
*/   

   glutCreateMenu(rightMenu);
   glutAddMenuEntry("Toggle Animation",1);
   glutAddMenuEntry("Start Game",2);
   glutAddMenuEntry("Quit",0);
   glutAttachMenu(GLUT_RIGHT_BUTTON);
}

// Routine to output interaction instructions to the C++ window.
void printInteraction(void)
{
   cout << "Interaction:" << endl;
   cout << "Press a to start the party" << endl;
   cout << "Right click to access the MENU" <<endl;
}

// Main routine.
int main(int argc, char **argv)
{
   printInteraction();
   glutInit(&argc, argv);
   glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH); 
   glutInitWindowSize (500, 500);
   glutInitWindowPosition (100, 100);
   glutCreateWindow ("spotlight.cpp");
   
   setup();
   
   glutDisplayFunc(drawScene);
   glutReshapeFunc(resize);
   glutKeyboardFunc(keyInput);
   glutSpecialFunc(specialKeyInput);
   
   glutTimerFunc(5, animate, 1);
   glutTimerFunc(5,rungame,1);
   makeMenu();
   
   glutMainLoop();
   
   return 0;
}
