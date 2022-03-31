  /////////////////////////
 // LIBRARY INCLUSIONS ///
/////////////////////////

#include <iostream> //For debug
#include <map> //Map map
#include <string.h> //Silly string
#include <filesystem> //For loading font files

#include <windows.h> //Platform dependent stuff

#include <glad/glad.h> //Bind OpenGL to functions
#include <GLFW/glfw3.h> //Connect OpenGL to GPU
#include <glm.hpp> //Math library
#include <ext/matrix_clip_space.hpp> //For glm::ortho
#include <gtc/type_ptr.hpp>
#include <ft2build.h> //Rendering text
#include FT_FREETYPE_H  //Basic FreeType functionality

#define STB_IMAGE_IMPLEMENTATION //Required by image reader to initialize correctly
#include <stb_image.h> //Load image reader for accessing textures

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h> //Just for test purposes



  /////////////
 // MACROS ///
/////////////

#ifndef RELEASE //Define RELEASE to compile as a release build
#define DEBUG std::cout //DEBUG(x) Sends messages to debug stream (terminal)
#define ERROR std::cerr //ERROR(x) Sends messages to error stream (terminal)
const char* Header = "    			%%#%%%%%%%%%%%%%%%+:::::\n			:===%%%%%%%%%%%%%%%%%%%%%%%%=\n		   =*%%%%%%%%%%%%%%%%%%%%%%%%%%%%===\n		   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n		   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n		   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n		   %%%%%%%%%%%#%%%%%%%%%%%%%%%%%%%%%\n		   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n		   %%%%%%%%%%%%%%%%%%%%%%@%%@@%%%%%%\n		   %%%%%%%%%%%%%%##%%%%%%@@@@@%%%%%%\n		   %%%%%%%%%%%%#*##%%%%%%%%%%%%%%%%%\n	 +++%%%%%%%%%%#@%%##*##**%%%%%%%%%%%%%%%+++\n   %%%%%%%%%%%%%%%%%%%%#***##*%%%%%%%%%%#%%%%%%%%\n =%%%%%%%%%%%%##%%%####**##**%%%%%%#%%***%%%%%%%#%%\n%%%%%%%%%%%%%###%%#######****#####***+*+*%%%%%%##%%##\n%%%%%%%%%%%%%%%%##%%%%*##*****+*+**+++++*%%%%##%%%%%%\n%%%%%%%%%%%%%%%%##%###*************+**++*%%%%##%%%%%%\n%%%%%%%%%%%%%%#####***********######**##%###**#%%%%%%\n%%%%%%%%%%%%%%%%%%%*****###%%####%%%**+++++++====##\n =%%%###%%%%%%%%%%%%%%%%%%%%%%%%%#*+++==*+++.::. --#\n :+%%**#%%%%%%%%###%%%%%%%%%%%%%%+++++==----:.     ::..\n   ==--%%%%#%%%%%%#%%%%%%%%%%%#%%#*=--======:.  .  ..::::\n	   ::::##%##%%#***%%###%%%%%%%+:**==::::::.  ....  :.:\n									  ***---  .+=        =\n										 +++-:...        +\n											=++--..::    :\n											 .-**++++....+-\n											   ------..::+*:..::--\n													   ...++++===-::=\n														 : .+=**++==-==**\n														 ...::==++*#*++======::-\n															..++--==+##++**==--=::==\n					   -*****+***+==-----                       ##++---++*#+*##+----==++\n				   *#*###*###*++++=---:::...+:                      #++----==++#**++++==..-\n				::*##*#****++=+++--=--..    .-=::                      ++-+=========++==--+--\n				##*+***++++++====--:::      ...==                         :::::-****==++--++*\n			  :=***++++++++++=:::.....         ..--                             ::##++++---::\n			-#+++*+++++++**:: ...::...           ..                               ++++++==-\n			-#*#++++=+==+::   .......            ..                            ++++++=++++:\n			-#    =+++   =*...----:...         ..                           :-*+==++-\n			.- @ *+*** @ %%-------:...   ......::..                    --::++=++*:\n			 *##******  =%#===---+-:....::-::..::-:         ..       ::++++++##+\n			  ********###**++----=-:::::-::::::::+-      -::..::  ==-++++**==:\n			  ****%%%%+==-----------==------==+--*  ***%%*##--==++++*##--\n				**%%%%===========-=+**===+++**#*-  =*##*#+======++--*\n				  *=+++++++++-===++===++****++   =***##**=----==::  -\n				  *+++===:::::---##*++++#--     -++######+----==::--=::..::\n				  +===---..::-+++#**==++++::   **####**%%*====**--##*##**====\n				  :===-==++**%****+=++++######*#####*%%***++++==-:--+++**%%%%==\n				   ::::=*     +**+++++**#---***%%##**##++###++++::++#++==%%%%%%:\n					       **+++++++++:::-==*****%%**==*****++::==+*=--**++==###\n					   -#**++++++++*     ******####++++##===..+=****-**++--==###\n						 **+++===*:      ###*####**--**==+====#*++---**::**%%*#=\n						 ==---=         #%%%*****++++++===::%#**==..+-:**%%##%%#\n	\n    ##### # # ###   ##  ### ###  ###   #  # ### # # ### ##    ### #  # ###   ##\n      #   # # #     # #  #  #  # #     ## # #   # # #   # #   #   ## # #  # #  \n      #   ### ###   ##   #  #  # ###   # ## ### # # ### ##    ### # ## #  #  #\n      #   # # #     # #  #  #  # #     # ## #    #  #   # #   #   # ## #  #   #\n      #   # # ###   # # ### ###  ###   #  # ###  #  ### # #   ### #  # ###  ##\n\n"; //MR. BONES
#else
#define DEBUG(x) //Do nothing with debug output on release builds
#define ERROR(X)
#endif



  ///////////////////
 // DECLARE TYPES //
///////////////////

struct Character {
	unsigned int ID; //ID number of glyph texture
	glm::ivec2 Size; //Width, height of glyph
	glm::ivec2 Bearing; //Offsets from baseline to left/top of glyph
	unsigned int Advance; //Horizontal distance to start of next glyph
};
std::map<unsigned int, Character> Alphabet; //Will hold a character for each loaded uint value- ergo, each needed Unicode character

struct TextField {
	std::vector<unsigned int> Chars; //List of codes representing the characters which make up the text field
	std::list<std::vector<unsigned char>> Formatting; //Each char represents a formatting change to apply to rendered characters, beginning with the character in the corresponding position of the Chars vector
	
	unsigned int MaxLength = 512; //How many characters allowed in a single text field (defaults to 512)
	unsigned char RowLength = 128; //How many characters to allow in a row (defaults to 128)
	float x, y; //Starting location of first character in first row
	float Scale = 1.0f; //Scale for characters, if no formatting modifies it
};





  //////////////////////////
 // INITIALIZE VARIABLES //
//////////////////////////

FT_Face face; //Create font face from file, !!! CAN NOT USE FREETYPE UNTIL IT IS INITIALIZED LATER !!!

const unsigned int xResolution = 1920; //Set window width
const unsigned int yResolution = 1080; //Set window height

unsigned int VAO, VBO; //Initialize VAO, VBO

const char *TextVSSource = "#version 460 core \n"
"layout (location = 0) in vec4 vertex;\n"
"out vec2 TexCoords;\n"
"uniform mat4 projection;\n"
"void main()\n"
"{\n"
"   gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);\n"
"   TexCoords = vertex.zw;\n"
"}\0";

const char *TextFSSource = "#version 460 core\n"
"in vec2 TexCoords;\n"
"out vec4 color;\n"
"uniform sampler2D text;\n"
"uniform vec3 textColor;\n"
"void main()\n"
"{\n"
"   vec4 sampled = vec4(1.0,1.0,1.0, texture(text,TexCoords).r);\n"
"   color = vec4(textColor, 1.0) * sampled;\n"
"}\n\0";


const char *CameraVSSource = "#version 460 core\n"
"layout (location = 0) in vec3 aPos;\n"
"layout (location = 1) in vec2 aTexCoord;\n"
"out vec2 TexCoords;\n"
"uniform mat4 model;\n"
"uniform mat4 view;\n"
"uniform mat4 projection;\n"
"void main()\n"
"{\n"
"   gl_Position = projection * view * model * vec4(aPos, 1.0f);\n"
"   TexCoords = vec2(aTexCoord.x, aTexCoord.y);\n"
"}\0";

const char *CameraFSSource = "#version 460 core\n"
"in vec2 TexCoords;\n"
"out vec4 FragColor;\n"
"uniform sampler2D cubetexture;\n"
"void main()\n"
"{\n"
"   FragColor = texture(cubetexture, TexCoords);\n"
"}\n\0";


float cube[] = {
		//BOTTOM FACE
	     0.5f,  0.5f, -0.5f,  1.0f, 1.0f, //5
		-0.5f,  0.5f, -0.5f,  65.0f / 96.0f, 1.0f, //6
		-0.5f, -0.5f, -0.5f,  65.0f / 96.0f, 0.0f, //7
	    
	    -0.5f, -0.5f, -0.5f,  65.0f / 96.0f, 0.0f, //7
		 0.5f, -0.5f, -0.5f,  1.0f, 0.0f, //8
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f, //5 

		//TOP FACE
		 0.5f,  0.5f,  0.5f,  32.0f / 96.0f, 1.0f, //1
		-0.5f,  0.5f,  0.5f,  0.0f, 1.0f, //2
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, //3
		
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, //3
		 0.5f, -0.5f,  0.5f,  32.0f / 96.0f, 0.0f, //4
		 0.5f,  0.5f,  0.5f,  32.0f / 96.0f, 1.0f, //1

		//WEST FACE
		-0.5f,  0.5f, -0.5f,  33.0f / 96.0f, 0.0f, //6
		-0.5f,  0.5f,  0.5f,  33.0f / 96.0f, 1.0f, //2
		-0.5f, -0.5f,  0.5f,  64.0f / 96.0f, 1.0f, //3

		-0.5f, -0.5f,  0.5f,  64.0f / 96.0f, 1.0f, //3
		-0.5f, -0.5f, -0.5f,  64.0f / 96.0f, 0.0f, //7
		-0.5f,  0.5f, -0.5f,  33.0f / 96.0f, 0.0f, //6

		//EAST FACE
		 0.5f,  0.5f,  0.5f,  64.0f / 96.0f, 1.0f, //1
		 0.5f,  0.5f, -0.5f,  64.0f / 96.0f, 0.0f, //5
		 0.5f, -0.5f, -0.5f,  33.0f / 96.0f, 0.0f, //8

		 0.5f, -0.5f, -0.5f,  33.0f / 96.0f, 0.0f, //8
		 0.5f, -0.5f,  0.5f,  33.0f / 96.0f, 1.0f, //4
		 0.5f,  0.5f,  0.5f,  64.0f / 96.0f, 1.0f, //1

		 //SOUTH FACE
		-0.5f, -0.5f, -0.5f,  33.0f / 96.0f, 0.0f, //7
		 0.5f, -0.5f, -0.5f,  64.0f / 96.0f, 0.0f, //8
		 0.5f, -0.5f,  0.5f,  64.0f / 96.0f, 1.0f, //4

		 0.5f, -0.5f,  0.5f,  64.0f / 96.0f, 1.0f, //4
		-0.5f, -0.5f,  0.5f,  33.0f / 96.0f, 1.0f, //3
		-0.5f, -0.5f, -0.5f,  33.0f / 96.0f, 0.0f, //7

		//NORTH FACE
		 0.5f,  0.5f, -0.5f,  33.0f / 96.0f, 0.0f, //5
		-0.5f,  0.5f, -0.5f,  64.0f / 96.0f, 0.0f, //6
		-0.5f,  0.5f,  0.5f,  64.0f / 96.0f, 1.0f, //2

	    -0.5f,  0.5f,  0.5f,  64.0f / 96.0f, 1.0f, //2
		 0.5f,  0.5f,  0.5f,  33.0f / 96.0f, 1.0f, //1
		 0.5f,  0.5f, -0.5f,  33.0f / 96.0f, 0.0f, //5
};

glm::vec3 cubeposition = {2.0f,0.0f,0.0f};

float dt = 0;
float FOV = 105.0f;
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 0.0f); 
glm::vec3 cameraFront = glm::vec3(1.0f, 0.0f, 0.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 0.0f, 1.0f);
bool debugHUD = false;
bool Wireframe = false;
bool last_LALT = false;
bool last_F1 = false;
bool last_F2 = false;
bool last_F3 = false;
bool RawMouse = true;
bool CaptureMouse = true;


bool firstMouse = true;

float yaw = 0.0f;
float pitch = 0.0f;
float lastX = xResolution / 2.0;
float lastY = yResolution / 2.0;

unsigned int new_character = 0;

bool KeyboardFocus = false;
bool new_text = false; 
bool KeyboardItalic = false;
bool KeyboardBold = false;
bool KeyboardUnderline = false;
bool KeyboardStrikethrough = false;

  /////////////////////////
 // FUNCTION DEFINITION //
/////////////////////////

void Get_Input(GLFWwindow *window) {
	#ifndef RELEASE
	
	if (glfwGetKey(window, GLFW_KEY_F1) == GLFW_RELEASE && last_F1) {
		debugHUD = !debugHUD;
	} //Every time F1 is pressed, toggle the visibility of the debug menu, if this is a debug build

	if (glfwGetKey(window, GLFW_KEY_F2) == GLFW_RELEASE && last_F2) {

		byte *pixels = new byte[3 * xResolution * yResolution]; //Allocate 3 bytes of memory on the heap for each pixel on the screen (one each for R, G, & B [this is probably a terrible way to do this, woops])
		glReadPixels(0, 0, xResolution, yResolution, GL_RGB, GL_BYTE, pixels); //Copy screen buffer pixels to memory
		stbi_flip_vertically_on_write(1);
		std::string screenshotname = std::to_string(glfwGetTime()) + ".png";
		stbi_write_png(screenshotname.c_str(), xResolution, yResolution, 3, pixels, 3 * xResolution); //Write jpg from memory
		delete pixels; //Screenshot in memory is a big object, so delete it immediately
	} //Every time F2 is pressed, take a screenshot

	if (glfwGetKey(window, GLFW_KEY_F3) == GLFW_RELEASE && last_F3) {
		if (!Wireframe) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		else {
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
		Wireframe = !Wireframe;
	} //Every time F3 is pressed, toggle Wireframe mode, if this is a debug build
	
	#endif


	if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_RELEASE && last_LALT) {
		if (CaptureMouse) {
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}
		else {
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		}
		CaptureMouse = !CaptureMouse;
	} //Every time Left Alt is pressed, capture/release mouse

	float cameraSpeed = static_cast<float>(2.5 * dt); 
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		cameraPos += cameraSpeed * cameraFront;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		cameraPos -= cameraSpeed * cameraFront;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		cameraPos += cameraSpeed * cameraUp;
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		cameraPos -= cameraSpeed * cameraUp;

	if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS) {
		last_LALT = true;
	}
	else {
		last_LALT = false;
	}

	if (glfwGetKey(window, GLFW_KEY_F1) == GLFW_PRESS) {
		last_F1 = true;
	}
	else {
		last_F1 = false;
	}

	if (glfwGetKey(window, GLFW_KEY_F2) == GLFW_PRESS) {
		last_F2 = true;
	}
	else {
		last_F2 = false;
	}

	if (glfwGetKey(window, GLFW_KEY_F3) == GLFW_PRESS) {
		last_F3 = true;
	}
	else {
		last_F3 = false;
	}
	
} //GLFW callback for handling keyboard input

void Get_Text(GLFWwindow* window, unsigned int codepoint) {
	
	unsigned int PHchar = FT_Get_Char_Index(face, codepoint); //Ensure that the UTF32 designation of the character translates to a character in the font face

	//Check to see if character has been loaded. If not, load it!
	if (Alphabet.find(PHchar) == Alphabet.end()) {

		FT_Load_Char(face, PHchar, FT_LOAD_RENDER); //Load character - FT_LOAD_RENDER results in an 8-bit grayscale bitmap being generated for the character

		unsigned int PHtexture;
		glGenTextures(1, &PHtexture); //Create font texture
		glBindTexture(GL_TEXTURE_2D, PHtexture); //Bind texture for processing
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, face->glyph->bitmap.width, face->glyph->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);

		//Set texture parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		//Store character into vector for later use
		Character character = { PHtexture,glm::ivec2(face->glyph->bitmap.width,face->glyph->bitmap.rows),glm::ivec2(face->glyph->bitmap_left,face->glyph->bitmap_top),static_cast<unsigned int>(face->glyph->advance.x) };
		Alphabet.insert(std::pair<unsigned int, Character>(PHchar, character)); //Insert created character and corresponding ASCII number into the map
	}

	new_character = PHchar; //Pass the character key value to a variable for handling
	new_text = true; //Flag that a new text character is waiting for use
} //GLFW callback for handling text entry

void Mouse_Callback(GLFWwindow* window, double xposIn, double yposIn) {
	float xpos = static_cast<float>(xposIn);
	float ypos = static_cast<float>(yposIn);

	if (firstMouse) {
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}
	if (CaptureMouse) {
		float xoffset = xpos - lastX;
		float yoffset = lastY - ypos; //y coordinates go from bottom to top
		lastX = xpos;
		lastY = ypos;

		float sensitivity = 0.1f;
		xoffset *= sensitivity;
		yoffset *= sensitivity;

		yaw -= xoffset;
		pitch += yoffset;

		//Bounds on mouselook freedom
		if (pitch > 89.9f)
			pitch = 89.9f;
		if (pitch < -89.9f)
			pitch = -89.9f;
		if (yaw > 180.0f)
			yaw -= 360.0f;
		if (yaw < -180.0f)
			yaw += 360.0f;

		glm::vec3 front; 
		front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		front.y = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		front.z = sin(glm::radians(pitch));
		
		cameraFront = glm::normalize(front);
	} //Only move camera with mouse if mouse is captured

} //GLFW callback for handling mouselook

void Scroll_Callback(GLFWwindow* window, double xoffset, double yoffset)
{
	FOV -= (float)yoffset;
	if (FOV < 1.0f)
		FOV = 1.0f;
	if (FOV > 120.0f)
		FOV = 120.0f;
} //GLFW callback for handling scroll wheel

void Window_Resize_Callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height); //When user resizes window, adjust viewport to match
} //Define callback for when user changes window size

void Error_Callback(int error, const char* description) {
	ERROR << Header << "GLFW Error: \n" << description;
} //Define callback for GLFW error message output

void OpenGL_Debug_Callback(GLenum source,GLenum type,GLuint id,GLenum severity,GLsizei length,const GLchar* message,const void* userParam) {
	switch (severity) {
		case GL_DEBUG_SEVERITY_HIGH:
			ERROR << Header << "OpenGL ERROR! - ";
			switch (source) {
				case GL_DEBUG_SOURCE_API:
					ERROR << "Source: API | ";
					break;
				case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
					ERROR << "Source: Window system API | ";
					break;
				case GL_DEBUG_SOURCE_SHADER_COMPILER:
					ERROR << "Source: Shader Compiler | ";
					break;
				case GL_DEBUG_SOURCE_THIRD_PARTY:
					ERROR << "Source: Third Party Application | ";
					break;
				case GL_DEBUG_SOURCE_APPLICATION:
					ERROR << "Source: This Application | ";
					break;
				case GL_DEBUG_SOURCE_OTHER:
					ERROR << "Source: Miscellaneous | ";
					break;
			}
			switch (type) {
			case GL_DEBUG_TYPE_ERROR:
				ERROR << "Type: Error | ";
				break;
			case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
				ERROR << "Type: Deprecated Behavior | ";
				break;
			case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
				ERROR << "Type: Undefined Behavior | ";
				break;
			case GL_DEBUG_TYPE_PORTABILITY:
				ERROR << "Type: Portability Warning | ";
				break;
			case GL_DEBUG_TYPE_PERFORMANCE:
				ERROR << "Type: Performance Warning | ";
				break;
			case GL_DEBUG_TYPE_MARKER:
				ERROR << "Type: Annotation | ";
				break;
			case GL_DEBUG_TYPE_PUSH_GROUP:
				ERROR << "Type: Group Pushing | ";
				break;
			case GL_DEBUG_TYPE_POP_GROUP:
				ERROR << "Type: Group Popping | ";
				break;
			case GL_DEBUG_TYPE_OTHER:
				ERROR << "Type: Miscellaneous | ";
				break;
			}
			ERROR << "Severity: HIGH | MESSAGE: " << message << "\n";

			exit(-1);
		default:
			DEBUG << "OpenGL - ";
			switch (source) {
			case GL_DEBUG_SOURCE_API:
				DEBUG << "Source: API | ";
				break;
			case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
				DEBUG << "Source: Window system API | ";
				break;
			case GL_DEBUG_SOURCE_SHADER_COMPILER:
				DEBUG << "Source: Shader Compiler | ";
				break;
			case GL_DEBUG_SOURCE_THIRD_PARTY:
				DEBUG << "Source: Third Party Application | ";
				break;
			case GL_DEBUG_SOURCE_APPLICATION:
				DEBUG << "Source: This Application | ";
				break;
			case GL_DEBUG_SOURCE_OTHER:
				DEBUG << "Source: Miscellaneous | ";
				break;
			}
			switch (type) {
			case GL_DEBUG_TYPE_ERROR:
				DEBUG << "Type: Error | ";
				break;
			case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
				DEBUG << "Type: Deprecated Behavior | ";
				break;
			case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
				DEBUG << "Type: Undefined Behavior | ";
				break;
			case GL_DEBUG_TYPE_PORTABILITY:
				DEBUG << "Type: Portability Warning | ";
				break;
			case GL_DEBUG_TYPE_PERFORMANCE:
				DEBUG << "Type: Performance Warning | ";
				break;
			case GL_DEBUG_TYPE_MARKER:
				DEBUG << "Type: Annotation | ";
				break;
			case GL_DEBUG_TYPE_PUSH_GROUP:
				DEBUG << "Type: Group Pushing | ";
				break;
			case GL_DEBUG_TYPE_POP_GROUP:
				DEBUG << "Type: Group Popping | ";
				break;
			case GL_DEBUG_TYPE_OTHER:
				DEBUG << "Type: Miscellaneous | ";
				break;
			}
			switch (severity) {
			case GL_DEBUG_SEVERITY_HIGH:
				DEBUG << "Severity: High | ";
				break;
			case GL_DEBUG_SEVERITY_MEDIUM:
				DEBUG << "Severity: Medium | ";
				break;
			case GL_DEBUG_SEVERITY_LOW:
				DEBUG << "Severity: Low | ";
				break;
			case GL_DEBUG_SEVERITY_NOTIFICATION:
				DEBUG << "Severity: Notification | ";
				break;
			}
			DEBUG << "MESSAGE: " << message << "\n";
	}
	
} //Define callback for OpenGL debug output

const char* getFreeTypeErrorMessage(FT_Error err) {
	#undef FTERRORS_H_
	#define FT_ERRORDEF( e, v, s )  case e: return s;
	#define FT_ERROR_START_LIST     switch (err) {
	#define FT_ERROR_END_LIST       }
	#include FT_ERRORS_H

	return "Shit! Unknown error!";
} //Function to extract error messages from FreeType functions

void RenderText(unsigned int TEXTSHADERPROGRAM, std::string text, float x, float y, float scale, glm::vec3 color) {
	
	glUseProgram(TEXTSHADERPROGRAM); //Set OpenGL to correct context
	glUniform3f(glGetUniformLocation(TEXTSHADERPROGRAM, "textColor"), color.x, color.y, color.z);
	glActiveTexture(GL_TEXTURE0); //Set active texture to first texture
	glBindVertexArray(VAO);

	glEnable(GL_BLEND); //Enable fragment blending (transparency)
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); //Select blend mode

	std::string::const_iterator c; //Sorcery
	for (c = text.begin(); c != text.end(); c++)
	{
  		Character ch = Alphabet[*c];

		float xpos = x + ch.Bearing.x * scale;
		float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

		float w = ch.Size.x * scale;
		float h = ch.Size.y * scale;
		
		
		float vertices[6][4] = {
			{ xpos,     ypos + h,   0.0f, 0.0f },
			{ xpos,     ypos,       0.0f, 1.0f },
			{ xpos + w, ypos,       1.0f, 1.0f },

			{ xpos,     ypos + h,   0.0f, 0.0f },
			{ xpos + w, ypos,       1.0f, 1.0f },
			{ xpos + w, ypos + h,   1.0f, 0.0f }
		}; //Set vertices to correct value for intended x, y
	
		glBindTexture(GL_TEXTURE_2D, ch.ID);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); //Send new data to VBO
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glDepthFunc(GL_ALWAYS); //Ignore depth test
		glDrawArrays(GL_TRIANGLES, 0, 6); //Draw... triangles? They're quads guys, trust me
		glDepthFunc(GL_LESS); //Re-enable depth test

		x += (ch.Advance >> 6) * scale; //Move target x value forward by the forward of the character just drawn (have to get it out of 1/64th pixels and into full pixels)
	}
	
	glDisable(GL_BLEND);
	
	glBindVertexArray(0); //Unbind VAO
	glBindTexture(GL_TEXTURE_2D, 0); //Unbind font texture
} //Draw text. Hopefully.

void RenderCharacter(unsigned int TEXTSHADERPROGRAM, unsigned int PHchar, float x, float y, float scale, glm::vec3 color) {

	glUseProgram(TEXTSHADERPROGRAM); //Set OpenGL to correct context
	glUniform3f(glGetUniformLocation(TEXTSHADERPROGRAM, "textColor"), color.x, color.y, color.z);
	glActiveTexture(GL_TEXTURE0); //Set active texture to first texture
	glBindVertexArray(VAO);

	glEnable(GL_BLEND); //Enable fragment blending (transparency)
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); //Select blend mode

		Character ch = Alphabet.at(PHchar);

		float xpos = x + ch.Bearing.x * scale;
		float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

		float w = ch.Size.x * scale;
		float h = ch.Size.y * scale;


		float vertices[6][4] = {
			{ xpos,     ypos + h,   0.0f, 0.0f },
			{ xpos,     ypos,       0.0f, 1.0f },
			{ xpos + w, ypos,       1.0f, 1.0f },

			{ xpos,     ypos + h,   0.0f, 0.0f },
			{ xpos + w, ypos,       1.0f, 1.0f },
			{ xpos + w, ypos + h,   1.0f, 0.0f }
		}; //Set vertices to correct value for intended x, y

		glBindTexture(GL_TEXTURE_2D, ch.ID);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); //Send new data to VBO
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glDepthFunc(GL_ALWAYS); //Ignore depth test
		glDrawArrays(GL_TRIANGLES, 0, 6); //Draw... triangles? They're quads guys, trust me
		glDepthFunc(GL_LESS); //Re-enable depth test

		x += (ch.Advance >> 6) * scale; //Move target x value forward by the forward of the character just drawn (have to get it out of 1/64th pixels and into full pixels)


	glDisable(GL_BLEND);

	glBindVertexArray(0); //Unbind VAO
	glBindTexture(GL_TEXTURE_2D, 0); //Unbind font texture
} //Draw text. Hopefully.



  ////  //  ////////////////////////////  //  ////
 ////  //  // !!! START FUNCTION !!! //  //  ////
////  //  ////////////////////////////  //  ////

int main() {

	  ///////////////////////////
	 // OpenGL Initialization //
	///////////////////////////

	glfwSetErrorCallback(Error_Callback); //Error callback may be set beforehand so intialization errors are captured
	if (!glfwInit()) //Initialize GLFW
	{
		ERROR << Header << "GLFW failed to initialize!\n"; //Error handling
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4); //Use OpenGL version 4.6
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); //Use OpenGL core profile (modern functions)

	GLFWwindow* GameWindow = glfwCreateWindow(xResolution, yResolution, "Artifice", NULL, NULL); //Create game window :D
	if (GameWindow == NULL) {
		ERROR << Header << "GLFW failed to create window! Terminating.\n"; //Error handling
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(GameWindow); //Point OpenGL at the game window
	glfwSetFramebufferSizeCallback(GameWindow, Window_Resize_Callback); //Enable window resize callback 
	glfwSetCursorPosCallback(GameWindow, Mouse_Callback); //Enable mouselook callback
	glfwSetScrollCallback(GameWindow, Scroll_Callback); //Enable scroll wheel callback

	glfwSetInputMode(GameWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED); //Capture mouse by default
	if (glfwRawMouseMotionSupported() && RawMouse) {
		glfwSetInputMode(GameWindow, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
	} //If raw mouse input is working, and the player hasn't disabled it, use it.

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) //Initialize GLAD- OpenGL functions should now be accessible
	{
		ERROR << Header << "GLAD failed to initialize! Terminating.\n"; //Error handling
		return -1;
	}
	
	#ifndef RELEASE //Turn on OpenGL debug logging if this is not a release build
		glEnable(GL_DEBUG_OUTPUT);
		glDebugMessageCallback(OpenGL_Debug_Callback, 0); //Attach debug output callback
	#endif

	glViewport(0, 0, xResolution, yResolution); //OpenGL viewport should take up the full screen space

	glEnable(GL_DEPTH_TEST); //Enable depth buffer
	



	int success;
	char infoLog[512];

      ////////////////////////////
	 // Text Rendering Shaders //
	////////////////////////////

	unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER); //Create empty vertex shader
	glShaderSource(vertexShader, 1, &TextVSSource, NULL); //Load vertex shader source code
	glCompileShader(vertexShader); //Compile vertex shader
	
	
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success); //Check to see if shader compiled properly
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog); //Error handling
		ERROR << Header << "Error! Vertex shader compilation failed with message:\n" << infoLog << "\n";
	}
	
	unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER); //Create empty fragment shader
	glShaderSource(fragmentShader, 1, &TextFSSource, NULL); //Load fragment shader source code
	glCompileShader(fragmentShader); //Compile fragment shader
	
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success); //Check to see if shader compiled properly
	if (!success)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog); //Error handling
		ERROR << Header << "Error! Fragment shader compilation failed with message:\n" << infoLog << "\n";
	}
	
	unsigned int shaderProgram = glCreateProgram(); //Create empty shader program
	glAttachShader(shaderProgram, vertexShader); //Attach shaders to program
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram); //Link shader program to GPU

	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success); //Check to see if linking executed properly
	if (!success) {
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog); //Error handling
		ERROR << Header << "Error! Shader linking failed with message:\n" << infoLog << "\n";
	}
	glDeleteShader(vertexShader); //Delete shaders (their contents are now a part of the shader program)
	glDeleteShader(fragmentShader);

	glm::mat4 ProjectionMatrix = glm::ortho(0.0f, static_cast<float>(xResolution), 0.0f, static_cast<float>(yResolution)); //Create orthogonal projection matrix (Places text quads in the correct position on-screen with OpenGL coordinates)

	glUseProgram(shaderProgram);

	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, 0, glm::value_ptr(ProjectionMatrix));
	//STUDY


	glGenVertexArrays(1, &VAO); //Create empty Vertex Array Object
	glGenBuffers(1, &VBO); //Create empty Vertex Buffer Object
	glBindVertexArray(VAO); //Bind VAO
	glBindBuffer(GL_ARRAY_BUFFER, VBO); //Bind VBO
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW); //Three coordinate and three color floats per vertex (6 total) * 4 vertexes (Text is made up of quads)
	glEnableVertexAttribArray(0); //Enable VAO
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0); 
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);



	  ////////////////////////////
     // Cube Rendering Shaders //
    ////////////////////////////

	unsigned int CameravertexShader = glCreateShader(GL_VERTEX_SHADER); //Create empty vertex shader
	glShaderSource(CameravertexShader, 1, &CameraVSSource, NULL); //Load vertex shader source code
	glCompileShader(CameravertexShader); //Compile vertex shader

	glGetShaderiv(CameravertexShader, GL_COMPILE_STATUS, &success); //Check to see if shader compiled properly
	if (!success)
	{
		glGetShaderInfoLog(CameravertexShader, 512, NULL, infoLog); //Error handling
		ERROR << Header << "Error! Vertex shader compilation failed with message:\n" << infoLog << "\n";
	}

	unsigned int CamerafragmentShader = glCreateShader(GL_FRAGMENT_SHADER); //Create empty fragment shader
	glShaderSource(CamerafragmentShader, 1, &CameraFSSource, NULL); //Load fragment shader source code
	glCompileShader(CamerafragmentShader); //Compile fragment shader

	glGetShaderiv(CamerafragmentShader, GL_COMPILE_STATUS, &success); //Check to see if shader compiled properly
	if (!success)
	{
		glGetShaderInfoLog(CamerafragmentShader, 512, NULL, infoLog); //Error handling
		ERROR << Header << "Error! Fragment shader compilation failed with message:\n" << infoLog << "\n";
	}

	unsigned int CamerashaderProgram = glCreateProgram(); //Create empty shader program
	glAttachShader(CamerashaderProgram, CameravertexShader); //Attach shaders to program
	glAttachShader(CamerashaderProgram, CamerafragmentShader);
	glLinkProgram(CamerashaderProgram); //Link shader program to GPU

	glGetProgramiv(CamerashaderProgram, GL_LINK_STATUS, &success); //Check to see if linking executed properly
	if (!success) {
		glGetProgramInfoLog(CamerashaderProgram, 512, NULL, infoLog); //Error handling
		ERROR << Header << "Error! Shader linking failed with message:\n" << infoLog << "\n";
	}
	
//#ifndef RELEASE //FORCE OUTPUT SHADER LOGS IF IN DEBUG
//	glGetShaderInfoLog(CameravertexShader, 512, NULL, infoLog);
//	DEBUG << "DEBUG | CAMERA VERTEX SHADER INFOLOG:\n" << infoLog << "\n\n";
//
//	glGetShaderInfoLog(CamerafragmentShader, 512, NULL, infoLog);
//	DEBUG << "DEBUG | CAMERA FRAGMENT SHADER INFOLOG:\n" << infoLog << "\n\n";
//
//	glGetProgramInfoLog(CamerashaderProgram, 512, NULL, infoLog);
//	DEBUG << "DEBUG | CAMERA SHADER PROGRAM INFOLOG:\n" << infoLog << "\n\n";
//#endif
	
	glDeleteShader(CameravertexShader); //Delete shaders (their contents are now a part of the shader program)
	glDeleteShader(CamerafragmentShader);



	unsigned int VBO1, VAO1;
	glGenVertexArrays(1, &VAO1);
	glGenBuffers(1, &VBO1);

	glBindVertexArray(VAO1);

	glBindBuffer(GL_ARRAY_BUFFER, VBO1);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cube), cube, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0); //For vertex coordinates
	glEnableVertexAttribArray(0);
	
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float))); //For texture coordinates
	glEnableVertexAttribArray(1);



	  /////////////////////////
	 // Import Cube Texture //
	/////////////////////////

	unsigned int cubetexture;
	glGenTextures(1, &cubetexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, cubetexture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); //Set texture wrapping mode
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); //Set texture filtering to linear
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	int width, height, nrChannels; //Resolution and color depth of loaded image
	stbi_set_flip_vertically_on_load(true); //Don't load the image upside down

	
	std::filesystem::path myPath69 = std::filesystem::current_path(); //Get working directory
	std::string myPath11 = myPath69.string(); //Convert working directory to string
	myPath11 += "\\textures\\grass.png"; //Append file location to working directory

	unsigned char *data = stbi_load(myPath11.c_str(), &width, &height, &nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		ERROR << Header << "Failed to load texture" << "\n";
	}

	
	stbi_image_free(data); //Return the memory of the raw texture data from file

	glUseProgram(CamerashaderProgram);
	glUniform1i(glGetUniformLocation(CamerashaderProgram, "cubetexture"), 1);



      /////////////////////////////
	 // FreeType Initialization //
	/////////////////////////////

	myPath69 = std::filesystem::current_path(); //Get working directory
	myPath11 = myPath69.string(); //Convert working directory to string
	myPath11 += "\\fonts\\CourierNew.ttf"; //Append file location to working directory

	FT_Library ft; //Load FreeType for text rendering
	if (FT_Init_FreeType(&ft)) {
		ERROR << Header << "Terminating! FreeType library failed to initialize with following message:\n " << getFreeTypeErrorMessage(FT_Init_FreeType(&ft)) << "\n";
		return -1;
	}

	if (FT_New_Face(ft, myPath11.c_str(), 0, &face)) {
		ERROR << Header << "Terminating! FreeType library failed to load font file with following message:\n " << getFreeTypeErrorMessage(FT_New_Face(ft, myPath11.c_str(), 0, &face)) << "\n" << myPath11.c_str() << "\n";
		return -1;
	}

	FT_Set_Pixel_Sizes(face, 0, 64); //Set character height to 64, default width
	
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1); //Reduce pixel size in memory down to 1 byte, since we only need one 8-bit number for each glyph bitmap pixel

	for (unsigned int c = 0; c < 128; c++) {
		if (FT_Load_Char(face, c, FT_LOAD_RENDER)) //Load character - FT_LOAD_RENDER results in an 8-bit grayscale bitmap being generated for the character
		{
			ERROR << Header << "Terminating! FreeType library failed to load glyph with following message:\n " << getFreeTypeErrorMessage(FT_Load_Char(face, c, FT_LOAD_RENDER)) << "\n";
			return -1;
		}

		unsigned int texture;
		glGenTextures(1, &texture); //Create font texture
		glBindTexture(GL_TEXTURE_2D, texture); //Bind texture for processing
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, face->glyph->bitmap.width, face->glyph->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);
		//STUDY

		//Set texture parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		//Store character into vector for later use
		Character character = { texture,glm::ivec2(face->glyph->bitmap.width,face->glyph->bitmap.rows),glm::ivec2(face->glyph->bitmap_left,face->glyph->bitmap_top),static_cast<unsigned int>(face->glyph->advance.x) };
		Alphabet.insert(std::pair<unsigned int, Character>(c, character)); //Insert created character and corresponding ASCII number into the map
	} 


	glBindTexture(GL_TEXTURE_2D, 0);

	//FT_Done_Face(face); //Stop using font face and return resources... probably can't do this if I want to dynamically stream in and out lots of unicode characters on the fly (multiplayer text chat)
	//FT_Done_FreeType(ft); //Stop using FreeType library and return resources


      ////////////////////
	 // Game Variables //
	////////////////////

	glm::vec3 messagergb;
	float t = 0.0f;
	float FPS;
	float rotationangle;
	
	std::string debugmessage;
	std::string debugcamerapos;
	std::string debugvideomode;
	std::string message = "was poppin";


	  ////  //  ///////////////////////  //  ////
	 ////  //  // !!! MAIN LOOP !!! //  //  ////
	////  //  ///////////////////////  //  ////

	while (!glfwWindowShouldClose(GameWindow)) //Run until window is closed
	{

		glClearColor(0.05f,0.1f,0.2f,1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //Set back buffer to solid dark sky color
		

		messagergb = {0.5*sin(t)+0.5f,0.5*sin(t * 1.5f)+0.5f,0.5*sin(t*0.5f)+0.5f}; //Set Text color to rainbow :D
		dt = glfwGetTime()-t;
		FPS = 1.0f / dt; //Find FPS
		t = glfwGetTime();

		//RENDER 3-D

		
		
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, cubetexture);
		glBindVertexArray(VAO1);
		
		glUseProgram(CamerashaderProgram); //Put OpenGL into the correct state
		

		glBindBuffer(GL_ARRAY_BUFFER, VBO1);

		//glm::mat4 projection = glm::mat4(1.0f);
		glm::mat4 projection = glm::perspective(glm::radians(FOV), (float)xResolution / (float)yResolution, 0.1f, 100.0f); //Construct projection matrix
		glUniformMatrix4fv(glGetUniformLocation(CamerashaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

		//glm::mat4 view = glm::mat4(1.0f);
		glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
		glUniformMatrix4fv(glGetUniformLocation(CamerashaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));

		

		glm::mat4 model = glm::mat4(1.0f); //Initialize matrix as an identity matrix
		model = glm::translate(model, cubeposition);
		rotationangle = glfwGetTime() / 0.2f;
		model = glm::rotate(model, glm::radians(rotationangle), glm::vec3(1.0f, 0.3f, 0.5f));
		glUniformMatrix4fv(glGetUniformLocation(CamerashaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

		

		glDrawArrays(GL_TRIANGLES, 0, 36);

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		//RENDER TEXT

		RenderText(shaderProgram, message, static_cast<float>(xResolution) * 1.5f / 10.0f, static_cast<float>(yResolution) * 5.0f / 10.0f, 1.0f, messagergb);

	#ifndef RELEASE
		if (debugHUD) {

			debugmessage = "ARTIFICE v0.1 " + std::to_string(FPS) + " FPS";
			RenderText(shaderProgram, debugmessage, static_cast<float>(xResolution) / 40.0f, static_cast<float>(yResolution) * 9.5f / 10.0f, 0.25f, glm::vec3{ 0.8f,0.8f,0.8f });
			debugcamerapos = "X: " + std::to_string(cameraPos.x) + "  Y: " + std::to_string(cameraPos.y) + "  Z: " + std::to_string(cameraPos.z) + "  Yaw: " + std::to_string(yaw) + "  Pitch: " + std::to_string(pitch);
			RenderText(shaderProgram, debugcamerapos, static_cast<float>(xResolution) / 40.0f, static_cast<float>(yResolution) * 9.25f / 10.0f, 0.25f, glm::vec3{ 0.8f,0.8f,0.8f });
			debugvideomode = std::to_string(xResolution) + " x " + std::to_string(yResolution);
			if (Wireframe) {
				debugvideomode += " WIREFRAME\n";
			}
			else {
				debugvideomode += "\n";
			}
			RenderText(shaderProgram, debugvideomode, static_cast<float>(xResolution) / 40.0f, static_cast<float>(yResolution) * 9.0f / 10.0f, 0.25f, glm::vec3{ 0.8f,0.8f,0.8f });
		}
	#endif

		//HANDLE INPUT

		Get_Input(GameWindow);

		glfwSwapBuffers(GameWindow); //Display new frame
		glfwPollEvents(); //Check for new events
	}

	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);

	glDeleteVertexArrays(1, &VAO1);
	glDeleteBuffers(1, &VBO1);

	glfwTerminate(); //Close out
	return 0;
}