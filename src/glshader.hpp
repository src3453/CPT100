// sharp bilinear filtering shader with post-processing effects

static const char * shader_vertex = R"(
#version 150
uniform vec2 scale;
uniform vec2 texDim;
in vec2 pos;
in vec2 texCoord;
smooth out vec2 texLoc;
void main()
{
    gl_Position = vec4(pos * scale, 0.0, 1.0);
    texLoc = texCoord * texDim;
}
)";

static const char * shader_fragment = R"(
#version 150
uniform sampler2D tex;
uniform vec2 texDim;
uniform vec2 screenSize;
uniform float blendBorder;
uniform float time;
smooth in vec2 texLoc;
out vec4 outputColor;

void main()
{
    vec2 slope = 0.5 / (blendBorder * fwidth(texLoc));
    vec2 subtexel = fract(texLoc);
    vec2 scaled = clamp(slope * subtexel, 0.0, 0.5) +
                  clamp(slope * (subtexel - 1.0) + 0.5, 0.0, 0.5);
    
    vec4 color = texture2D(tex, (floor(texLoc) + scaled) / texDim);
    
    // Post-processing effects can be added here
    outputColor = color;
}
)";

class GLShaderProgram {
public:
    GLuint program;
    GLuint vao;
    GLuint vbo_pos, vbo_tex;
    
    GLShaderProgram() : program(0), vao(0), vbo_pos(0), vbo_tex(0) {}
    
    bool compile() {
        // Compile vertex shader
        GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &shader_vertex, NULL);
        glCompileShader(vertexShader);
        
        // Check vertex shader compile status
        int success;
        char infoLog[512];
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
            std::cerr << "Vertex shader compilation failed: " << infoLog << std::endl;
            return false;
        }
        
        // Compile fragment shader
        GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &shader_fragment, NULL);
        glCompileShader(fragmentShader);
        
        // Check fragment shader compile status
        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
            std::cerr << "Fragment shader compilation failed: " << infoLog << std::endl;
            return false;
        }
        
        // Link program
        program = glCreateProgram();
        glAttachShader(program, vertexShader);
        glAttachShader(program, fragmentShader);
        glBindAttribLocation(program, 0, "pos");
        glBindAttribLocation(program, 1, "texCoord");
        glLinkProgram(program);
        
        // Check link status
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(program, 512, NULL, infoLog);
            std::cerr << "Shader program linking failed: " << infoLog << std::endl;
            return false;
        }
        
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        
        return true;
    }
    
    void setupVertexData() {
        // Setup VAO and VBO for fullscreen quad
        GLfloat vertices[] = {
            -1.0f, -1.0f,
             1.0f, -1.0f,
             1.0f,  1.0f,
            -1.0f,  1.0f
        };
        
        GLfloat texCoords[] = {
            0.0f, 1.0f,
            1.0f, 1.0f,
            1.0f, 0.0f,
            0.0f, 0.0f
        };
        
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        
        // Position VBO
        glGenBuffers(1, &vbo_pos);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_pos);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*)0);
        
        // TexCoord VBO
        glGenBuffers(1, &vbo_tex);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_tex);
        glBufferData(GL_ARRAY_BUFFER, sizeof(texCoords), texCoords, GL_STATIC_DRAW);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*)0);
        
        glBindVertexArray(0);
    }
    
    void use() {
        glUseProgram(program);
    }
    
    void setUniform2f(const char* name, float x, float y) {
        GLint loc = glGetUniformLocation(program, name);
        glUniform2f(loc, x, y);
    }
    
    void setUniform1f(const char* name, float value) {
        GLint loc = glGetUniformLocation(program, name);
        glUniform1f(loc, value);
    }
    
    void setUniform1i(const char* name, int value) {
        GLint loc = glGetUniformLocation(program, name);
        glUniform1i(loc, value);
    }
    
    void renderQuad() {
        glBindVertexArray(vao);
        glDrawArrays(GL_QUADS, 0, 4);
        glBindVertexArray(0);
    }
    
    void cleanup() {
        if (vao) glDeleteVertexArrays(1, &vao);
        if (vbo_pos) glDeleteBuffers(1, &vbo_pos);
        if (vbo_tex) glDeleteBuffers(1, &vbo_tex);
        if (program) glDeleteProgram(program);
    }
};