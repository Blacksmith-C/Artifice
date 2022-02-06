#include <glad/glad.h> //Bind OpenGL to functions
#include <GLFW/glfw3.h> //Connect OpenGL to GPU
#include <glm.hpp> //Math library
#include <ft2build.h> //Rendering text
#include FT_FREETYPE_H  //Basic FreeType functionality
#include <iostream> //For debug
#include <map> //Map map
#include <string> //Silly string

#ifndef RELEASE //Define RELEASE to compile as a release build
#define DEBUG(x) std::cout << x << "\n" //DEBUG(x) Sends debug message x to terminal
#define ERROR(x) std::cerr << "    			%%#%%%%%%%%%%%%%%%+:::::\n			:===%%%%%%%%%%%%%%%%%%%%%%%%=\n		   =*%%%%%%%%%%%%%%%%%%%%%%%%%%%%===\n		   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n		   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n		   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n		   %%%%%%%%%%%#%%%%%%%%%%%%%%%%%%%%%\n		   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n		   %%%%%%%%%%%%%%%%%%%%%%@%%@@%%%%%%\n		   %%%%%%%%%%%%%%##%%%%%%@@@@@%%%%%%\n		   %%%%%%%%%%%%#*##%%%%%%%%%%%%%%%%%\n	 +++%%%%%%%%%%#@%%##*##**%%%%%%%%%%%%%%%+++\n   %%%%%%%%%%%%%%%%%%%%#***##*%%%%%%%%%%#%%%%%%%%\n =%%%%%%%%%%%%##%%%####**##**%%%%%%#%%***%%%%%%%#%%\n%%%%%%%%%%%%%###%%#######****#####***+*+*%%%%%%##%%##\n%%%%%%%%%%%%%%%%##%%%%*##*****+*+**+++++*%%%%##%%%%%%\n%%%%%%%%%%%%%%%%##%###*************+**++*%%%%##%%%%%%\n%%%%%%%%%%%%%%#####***********######**##%###**#%%%%%%\n%%%%%%%%%%%%%%%%%%%*****###%%####%%%**+++++++====##\n =%%%###%%%%%%%%%%%%%%%%%%%%%%%%%#*+++==*+++.::. --#\n :+%%**#%%%%%%%%###%%%%%%%%%%%%%%+++++==----:.     ::..\n   ==--%%%%#%%%%%%#%%%%%%%%%%%#%%#*=--======:.  .  ..::::\n	   ::::##%##%%#***%%###%%%%%%%+:**==::::::.  ....  :.:\n									  ***---  .+=        =\n										 +++-:...        +\n											=++--..::    :\n											 .-**++++....+-\n											   ------..::+*:..::--\n													   ...++++===-::=\n														 : .+=**++==-==**\n														 ...::==++*#*++======::-\n															..++--==+##++**==--=::==\n					   -*****+***+==-----                       ##++---++*#+*##+----==++\n				   *#*###*###*++++=---:::...+:                      #++----==++#**++++==..-\n				::*##*#****++=+++--=--..    .-=::                      ++-+=========++==--+--\n				##*+***++++++====--:::      ...==                         :::::-****==++--++*\n			  :=***++++++++++=:::.....         ..--                             ::##++++---::\n			-#+++*+++++++**:: ...::...           ..                               ++++++==-\n			-#*#++++=+==+::   .......            ..                            ++++++=++++:\n			-#    =+++   =*...----:...         ..                           :-*+==++-\n			.- @ *+*** @ %%-------:...   ......::..                    --::++=++*:\n			 *##******  =%#===---+-:....::-::..::-:         ..       ::++++++##+\n			  ********###**++----=-:::::-::::::::+-      -::..::  ==-++++**==:\n			  ****%%%%+==-----------==------==+--*  ***%%*##--==++++*##--\n				**%%%%===========-=+**===+++**#*-  =*##*#+======++--*\n				  *=+++++++++-===++===++****++   =***##**=----==::  -\n				  *+++===:::::---##*++++#--     -++######+----==::--=::..::\n				  +===---..::-+++#**==++++::   **####**%%*====**--##*##**====\n				  :===-==++**%****+=++++######*#####*%%***++++==-:--+++**%%%%==\n				   ::::=*     +**+++++**#---***%%##**##++###++++::++#++==%%%%%%:\n					       **+++++++++:::-==*****%%**==*****++::==+*=--**++==###\n					   -#**++++++++*     ******####++++##===..+=****-**++--==###\n						 **+++===*:      ###*####**--**==+====#*++---**::**%%*#=\n						 ==---=         #%%%*****++++++===::%#**==..+-:**%%##%%#\n	\n    ##### # # ###   ##  ### ###  ###   #  # ### # # ### ##    ### #  # ###   ##\n      #   # # #     # #  #  #  # #     ## # #   # # #   # #   #   ## # #  # #  \n      #   ### ###   ##   #  #  # ###   # ## ### # # ### ##    ### # ## #  #  #\n      #   # # #     # #  #  #  # #     # ## #    #  #   # #   #   # ## #  #   #\n      #   # # ###   # # ### ###  ###   #  # ###  #  ### # #   ### #  # ###  ##\n\n" << x << "\n" //ERROR(x) Sends error message x to terminal
#undef FTERRORS_H_
#define FT_ERROR_START_LIST     switch ( error_code ) {
#define FT_ERRORDEF( e, v, s )    case v: return s;
#define FT_ERROR_END_LIST       }

#else
#define DEBUG(x) //Do nothing with debug output on release builds
#define ERROR(X)
#endif



//#define STB_IMAGE_IMPLEMENTATION //Required by image reader to initialize correctly
//#include <C:\Artifice\GLtest\stb_image.h> //Load image reader for accessing textures

struct Character {
	unsigned int ID; //ID number of glyph texture
	glm::ivec2 Size; //Width, height of glyph
	glm::ivec2 Bearing; //Offsets from baseline to left/top of glyph
	unsigned int Advance; //Horizontal distance to start of next glyph
};

void Window_Resize_Callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height); //When user resizes window, adjust viewport to match
} //Define callback for when user changes window size

void Error_Callback(int error, const char* description) {
	std::cerr << "GLFW Error: \n" << description;
} //Define callback for GLFW error message output

const char* getFreeTypeErrorMessage(FT_Error err)
{
#undef FTERRORS_H_
#define FT_ERRORDEF( e, v, s )  case e: return s;
#define FT_ERROR_START_LIST     switch (err) {
#define FT_ERROR_END_LIST       }
#include FT_ERRORS_H
	return "(Unknown error)";
}

const unsigned int xResolution = 1920; //Set window width
const unsigned int yResolution = 1080; //Set window height



int main() {

	  ///////////////////////////
	 // OpenGL Initialization //
	///////////////////////////

	glfwSetErrorCallback(Error_Callback); //Error callback may be set beforehand so intialization errors are captured
	if (!glfwInit()) //Initialize GLFW
	{
		ERROR("GLFW failed to initialize!"); //Error handling
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4); //Use OpenGL version 4.6
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); //Use OpenGL core profile (modern functions)

	GLFWwindow* GameWindow = glfwCreateWindow(xResolution, yResolution, "Artifice", NULL, NULL); //Create game window :D
	if (GameWindow == NULL) {
		ERROR("GLFW failed to create window! Terminating."); //Error handling
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(GameWindow); //Point OpenGL at the game window
	glfwSetFramebufferSizeCallback(GameWindow, Window_Resize_Callback); //Enable window resize callback 

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) //Initialize GLAD- OpenGL functions should now be accessible
	{
		ERROR("GLAD failed to initialize! Terminating."); //Error handling
		return -1;
	}
	glViewport(0, 0, xResolution, yResolution); //OpenGL viewport should take up the full screen space


      /////////////////////////////
	 // FreeType Initialization //
	/////////////////////////////

	FT_Library ft; //Load FreeType for text rendering
	if (FT_Init_FreeType(&ft)) {
		ERROR("FreeType library failed to initialize! Terminating.");
		std::cerr << getFreeTypeErrorMessage(FT_Init_FreeType(&ft));
		return -1;
	}

	FT_Face face; //Create font face from file
	if (FT_New_Face(ft, "fonts\OpenSans-Regular.ttf", 0, &face)) {
		ERROR("FreeType failed to load font file! Terminating.");
		std::cerr << getFreeTypeErrorMessage(FT_New_Face(ft, "fonts\OpenSans-Regular.ttf", 0, &face));
		return -1;
	}

	FT_Set_Pixel_Sizes(face, 0, 64); //Set character height to 64, default width
	
	std::map<char, Character> Characters; //Create a character for each possible char value- ergo, each ASCII character
	
	//unsigned int VAO, VBO; //Create Vertex attribute, vertex buffer objects

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1); //Reduce pixel size in memory down to 1 byte, since we only need one 8-bit number for each glyph bitmap pixel

	for (unsigned char c = 0; c < 128; c++) {
		if (FT_Load_Char(face, c, FT_LOAD_RENDER)) //Load character - FT_LOAD_RENDER results in an 8-bit grayscale bitmap being generated for the character
		{
			ERROR("FreeType failed to load glyph! Terminating.");
			std::cerr << FT_Load_Char(face, c, FT_LOAD_RENDER);
			return -1;
		}

		unsigned int texture;
		glGenTextures(1, &texture); //Create font texture
		glBindTexture(GL_TEXTURE_2D, texture); //Bind texture for processing
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, face->glyph->bitmap.width, face->glyph->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);
		// I NEED TO BREAK DOWN THIS COMMAND AND UNDERSTAND IT FULLY

		//Set texture parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		//Store character into vector for later use
		Character character = { texture,glm::ivec2(face->glyph->bitmap.width,face->glyph->bitmap.rows),glm::ivec2(face->glyph->bitmap_left,face->glyph->bitmap_top),static_cast<unsigned int>(face->glyph->advance.x) };
		Characters.insert(std::pair<char, Character>(c, character)); //Insert created character and corresponding ASCII number into the map
	}
	glBindTexture(GL_TEXTURE_2D, 0);

	FT_Done_Face(face); //Tell FreeType we're done with the font face
	FT_Done_FreeType(ft); //Tell FreeType we're done with the library



	


	  ////  //  ///////////////////////  //  ////
	 ////  //  // !!! MAIN LOOP !!! //  //  ////
	////  //  ///////////////////////  //  ////

	while (!glfwWindowShouldClose(GameWindow)) //Run until window is closed
	{

		glClearColor(0.1f,0.15f,0.4f,1.0f);
		glClear(GL_COLOR_BUFFER_BIT); //Set back buffer to solid dark sky color

		glfwSwapBuffers(GameWindow); //Display new frame
		glfwPollEvents(); //Check for new events


	}


	glfwTerminate(); //Close out
	return 0;
}