#include <iostream> //For debug
#include <map> //Map map
#include <string> //Silly string
#include <filesystem> //For loading font files

#include <glad/glad.h> //Bind OpenGL to functions
#include <GLFW/glfw3.h> //Connect OpenGL to GPU
#include <glm.hpp> //Math library
#include <ext/matrix_clip_space.hpp> //For glm::ortho
#include <gtc/type_ptr.hpp>
#include <ft2build.h> //Rendering text
#include FT_FREETYPE_H  //Basic FreeType functionality

#ifndef RELEASE //Define RELEASE to compile as a release build
#define DEBUG std::cout //DEBUG(x) Sends messages to debug stream (terminal)
#define ERROR std::cerr //ERROR(x) Sends messages to error stream (terminal)
const char* Header = "    			%%#%%%%%%%%%%%%%%%+:::::\n			:===%%%%%%%%%%%%%%%%%%%%%%%%=\n		   =*%%%%%%%%%%%%%%%%%%%%%%%%%%%%===\n		   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n		   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n		   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n		   %%%%%%%%%%%#%%%%%%%%%%%%%%%%%%%%%\n		   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n		   %%%%%%%%%%%%%%%%%%%%%%@%%@@%%%%%%\n		   %%%%%%%%%%%%%%##%%%%%%@@@@@%%%%%%\n		   %%%%%%%%%%%%#*##%%%%%%%%%%%%%%%%%\n	 +++%%%%%%%%%%#@%%##*##**%%%%%%%%%%%%%%%+++\n   %%%%%%%%%%%%%%%%%%%%#***##*%%%%%%%%%%#%%%%%%%%\n =%%%%%%%%%%%%##%%%####**##**%%%%%%#%%***%%%%%%%#%%\n%%%%%%%%%%%%%###%%#######****#####***+*+*%%%%%%##%%##\n%%%%%%%%%%%%%%%%##%%%%*##*****+*+**+++++*%%%%##%%%%%%\n%%%%%%%%%%%%%%%%##%###*************+**++*%%%%##%%%%%%\n%%%%%%%%%%%%%%#####***********######**##%###**#%%%%%%\n%%%%%%%%%%%%%%%%%%%*****###%%####%%%**+++++++====##\n =%%%###%%%%%%%%%%%%%%%%%%%%%%%%%#*+++==*+++.::. --#\n :+%%**#%%%%%%%%###%%%%%%%%%%%%%%+++++==----:.     ::..\n   ==--%%%%#%%%%%%#%%%%%%%%%%%#%%#*=--======:.  .  ..::::\n	   ::::##%##%%#***%%###%%%%%%%+:**==::::::.  ....  :.:\n									  ***---  .+=        =\n										 +++-:...        +\n											=++--..::    :\n											 .-**++++....+-\n											   ------..::+*:..::--\n													   ...++++===-::=\n														 : .+=**++==-==**\n														 ...::==++*#*++======::-\n															..++--==+##++**==--=::==\n					   -*****+***+==-----                       ##++---++*#+*##+----==++\n				   *#*###*###*++++=---:::...+:                      #++----==++#**++++==..-\n				::*##*#****++=+++--=--..    .-=::                      ++-+=========++==--+--\n				##*+***++++++====--:::      ...==                         :::::-****==++--++*\n			  :=***++++++++++=:::.....         ..--                             ::##++++---::\n			-#+++*+++++++**:: ...::...           ..                               ++++++==-\n			-#*#++++=+==+::   .......            ..                            ++++++=++++:\n			-#    =+++   =*...----:...         ..                           :-*+==++-\n			.- @ *+*** @ %%-------:...   ......::..                    --::++=++*:\n			 *##******  =%#===---+-:....::-::..::-:         ..       ::++++++##+\n			  ********###**++----=-:::::-::::::::+-      -::..::  ==-++++**==:\n			  ****%%%%+==-----------==------==+--*  ***%%*##--==++++*##--\n				**%%%%===========-=+**===+++**#*-  =*##*#+======++--*\n				  *=+++++++++-===++===++****++   =***##**=----==::  -\n				  *+++===:::::---##*++++#--     -++######+----==::--=::..::\n				  +===---..::-+++#**==++++::   **####**%%*====**--##*##**====\n				  :===-==++**%****+=++++######*#####*%%***++++==-:--+++**%%%%==\n				   ::::=*     +**+++++**#---***%%##**##++###++++::++#++==%%%%%%:\n					       **+++++++++:::-==*****%%**==*****++::==+*=--**++==###\n					   -#**++++++++*     ******####++++##===..+=****-**++--==###\n						 **+++===*:      ###*####**--**==+====#*++---**::**%%*#=\n						 ==---=         #%%%*****++++++===::%#**==..+-:**%%##%%#\n	\n    ##### # # ###   ##  ### ###  ###   #  # ### # # ### ##    ### #  # ###   ##\n      #   # # #     # #  #  #  # #     ## # #   # # #   # #   #   ## # #  # #  \n      #   ### ###   ##   #  #  # ###   # ## ### # # ### ##    ### # ## #  #  #\n      #   # # #     # #  #  #  # #     # ## #    #  #   # #   #   # ## #  #   #\n      #   # # ###   # # ### ###  ###   #  # ###  #  ### # #   ### #  # ###  ##\n\n"; //MR. BONES
#else
#define DEBUG(x) //Do nothing with debug output on release builds
#define ERROR(X)
#endif

//#define STB_IMAGE_IMPLEMENTATION //Required by image reader to initialize correctly
//#include <C:\Artifice\GLtest\stb_image.h> //Load image reader for accessing textures



const unsigned int xResolution = 1920; //Set window width
const unsigned int yResolution = 1080; //Set window height
unsigned int VAO, VBO; //Initialize VAO, VBO

const char *fontname = "fonts/CourierNew.ttf";

const char *vertexShaderSource = "#version 460 core\n"
"layout (location = 0) in vec4 vertex;\n"
"out vec2 TexCoords;\n"
"uniform mat4 projection;\n"
"void main()\n"
"{\n"
"   gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);\n"
"   TexCoords = vertex.zw;\n"
"}\0";

const char *fragmentShaderSource = "#version 460 core\n"
"in vec2 TexCoords;\n"
"out vec4 color;\n"
"uniform sampler2D text;\n"
"uniform vec3 textColor;\n"
"void main()\n"
"{\n"
"   vec4 sampled = vec4(1.0,1.0,1.0, texture(text,TexCoords).r);\n"
"   color = vec4(textColor, 1.0) * sampled;\n"
"}\n\0";


struct Character {
	unsigned int ID; //ID number of glyph texture
	glm::ivec2 Size; //Width, height of glyph
	glm::ivec2 Bearing; //Offsets from baseline to left/top of glyph
	unsigned int Advance; //Horizontal distance to start of next glyph
};
std::map<char, Character> Characters; //Will hold a character for each possible char value- ergo, each ASCII character

void Window_Resize_Callback(GLFWwindow* window, int width, int height)
{
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

	std::string::const_iterator c; //Sorcery
	for (c = text.begin(); c != text.end(); c++)
	{
  		Character ch = Characters[*c];

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

		glDrawArrays(GL_TRIANGLES, 0, 6); //Draw... triangles? It's a quad guys, trust me

		x += (ch.Advance >> 6) * scale; //Move target x value forward by the forward of the character just drawn (have to get it out of 1/64th pixels and into full pixels)
	}
	glBindVertexArray(0); //Unbind VAO
	glBindTexture(GL_TEXTURE_2D, 0); //Unbind font texture
} //Draw text. Hopefully.



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

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) //Initialize GLAD- OpenGL functions should now be accessible
	{
		ERROR << Header << "GLAD failed to initialize! Terminating.\n"; //Error handling
		return -1;
	}
	
	glEnable(GL_DEBUG_OUTPUT); //Turn on OpenGL debug logging
	glDebugMessageCallback(OpenGL_Debug_Callback, 0);
	glViewport(0, 0, xResolution, yResolution); //OpenGL viewport should take up the full screen space

	glEnable(GL_BLEND); //Enable fragment blending (transparency)
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); //Select blend mode

      ////////////////////////////
	 // Text Rendering Shaders //
	////////////////////////////

	unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER); //Create empty vertex shader
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL); //Load vertex shader source code
	glCompileShader(vertexShader); //Compile vertex shader
	
	int success;
	char infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success); //Check to see if shader compiled properly
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog); //Error handling
		ERROR << Header << "Error! Vertex shader compilation failed with message:\n" << infoLog << "\n";
	}
	
	unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER); //Create empty fragment shader
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL); //Load fragment shader source code
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

	  ////////////////////////////
     // Text Rendering Shaders //
    ////////////////////////////

	glGenVertexArrays(1, &VAO); //Create empty Vertex Array Object
	glGenBuffers(1, &VBO); //Create empty Vertex Buffer Object
	glBindVertexArray(VAO); //Bind VAO
	glBindBuffer(GL_ARRAY_BUFFER, VBO); //Bind VBO
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW); //Three coordinate and three color floats per vertex (6 total) * 4 vertexes (Text is made up of quads)
	glEnableVertexAttribArray(0); //Enable VAO
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0); 
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);


      /////////////////////////////
	 // FreeType Initialization //
	/////////////////////////////

	FT_Library ft; //Load FreeType for text rendering
	if (FT_Init_FreeType(&ft)) {
		ERROR << Header << "Terminating! FreeType library failed to initialize with following message:\n " << getFreeTypeErrorMessage(FT_Init_FreeType(&ft)) << "\n";
		return -1;
	}


	FT_Face face; //Create font face from file
	if (FT_New_Face(ft, fontname, 0, &face)) {
		ERROR << Header << "Terminating! FreeType library failed to load font file with following message:\n " << getFreeTypeErrorMessage(FT_New_Face(ft, fontname, 0, &face)) << "\n";
		return -1;
	}

	FT_Set_Pixel_Sizes(face, 0, 64); //Set character height to 64, default width
	
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1); //Reduce pixel size in memory down to 1 byte, since we only need one 8-bit number for each glyph bitmap pixel

	for (unsigned char c = 0; c < 128; c++) {
		if (FT_Load_Char(face, c, FT_LOAD_RENDER)) //Load character - FT_LOAD_RENDER results in an 8-bit grayscale bitmap being generated for the character
		{
			ERROR << Header << "Terminating! FreeType library failed to load glyph with following message:\n " << getFreeTypeErrorMessage(FT_Load_Char(face, c, FT_LOAD_RENDER)) << "\n";
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

	FT_Done_Face(face); //Stop using font face and return resources
	FT_Done_FreeType(ft); //Stop using FreeType library and return resources


      ////////////////////
	 // Game Variables //
	////////////////////

	glm::vec3 messagergb;
	float t = 0.0f;
	float dt;
	float FPS;
	bool debugmenu = true;
	std::string debugmessage;
	std::string message = "I'm in love with Michelle Passmore";


	  ////  //  ///////////////////////  //  ////
	 ////  //  // !!! MAIN LOOP !!! //  //  ////
	////  //  ///////////////////////  //  ////

	while (!glfwWindowShouldClose(GameWindow)) //Run until window is closed
	{

		glClearColor(0.05f,0.1f,0.2f,1.0f);
		glClear(GL_COLOR_BUFFER_BIT); //Set back buffer to solid dark sky color
		
		messagergb = {0.5*sin(t)+0.5f,0.5*sin(t * 1.5f)+0.5f,0.5*sin(t*0.5f)+0.5f}; //Set Text color to rainbow :D
		dt = glfwGetTime()-t;
		FPS = 1.0f / dt; //Find FPS
		t = glfwGetTime();

		RenderText(shaderProgram, message, static_cast<float>(xResolution) * 1.5f / 10.0f, static_cast<float>(yResolution) * 5.0f / 10.0f, 1.0f, messagergb);

		if (debugmenu) {

			debugmessage = "ARTIFICE v0 " + std::to_string(FPS) + " FPS";
			RenderText(shaderProgram, debugmessage, static_cast<float>(xResolution) / 40.0f, static_cast<float>(yResolution) * 9.5f / 10.0f, 0.25f, glm::vec3{ 0.8f,0.8f,0.8f });
		}
		
		glfwSwapBuffers(GameWindow); //Display new frame
		glfwPollEvents(); //Check for new events


	}


	glfwTerminate(); //Close out
	return 0;
}