/*
 * Minimal OpenGL 3.3 Core loader for MinGW/Windows
 * Generated-style header - only includes functions needed for this demo
 */
#ifndef GLAD_H
#define GLAD_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <GL/gl.h>

/* ---- Typedefs for OpenGL 3.x+ functions ---- */

/* Shader types */
typedef char GLchar;
typedef ptrdiff_t GLintptr;
typedef ptrdiff_t GLsizeiptr;

/* VAO / VBO */
typedef void  (APIENTRY *PFNGLGENVERTEXARRAYSPROC)(GLsizei n, GLuint *arrays);
typedef void  (APIENTRY *PFNGLBINDVERTEXARRAYPROC)(GLuint array);
typedef void  (APIENTRY *PFNGLDELETEVERTEXARRAYSPROC)(GLsizei n, const GLuint *arrays);
typedef void  (APIENTRY *PFNGLGENBUFFERSPROC)(GLsizei n, GLuint *buffers);
typedef void  (APIENTRY *PFNGLBINDBUFFERPROC)(GLenum target, GLuint buffer);
typedef void  (APIENTRY *PFNGLBUFFERDATAPROC)(GLenum target, GLsizeiptr size, const void *data, GLenum usage);
typedef void  (APIENTRY *PFNGLDELETEBUFFERSPROC)(GLsizei n, const GLuint *buffers);
typedef void  (APIENTRY *PFNGLENABLEVERTEXATTRIBARRAYPROC)(GLuint index);
typedef void  (APIENTRY *PFNGLVERTEXATTRIBPOINTERPROC)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);
typedef void  (APIENTRY *PFNGLVERTEXATTRIBDIVISORPROC)(GLuint index, GLuint divisor);

/* Shaders */
typedef GLuint (APIENTRY *PFNGLCREATESHADERPROC)(GLenum type);
typedef void   (APIENTRY *PFNGLSHADERSOURCEPROC)(GLuint shader, GLsizei count, const GLchar *const *string, const GLint *length);
typedef void   (APIENTRY *PFNGLCOMPILESHADERPROC)(GLuint shader);
typedef void   (APIENTRY *PFNGLDELETESHADERPROC)(GLuint shader);
typedef void   (APIENTRY *PFNGLGETSHADERIVPROC)(GLuint shader, GLenum pname, GLint *params);
typedef void   (APIENTRY *PFNGLGETSHADERINFOLOGPROC)(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);

/* Programs */
typedef GLuint (APIENTRY *PFNGLCREATEPROGRAMPROC)(void);
typedef void   (APIENTRY *PFNGLATTACHSHADERPROC)(GLuint program, GLuint shader);
typedef void   (APIENTRY *PFNGLLINKPROGRAMPROC)(GLuint program);
typedef void   (APIENTRY *PFNGLUSEPROGRAMPROC)(GLuint program);
typedef void   (APIENTRY *PFNGLDELETEPROGRAMPROC)(GLuint program);
typedef void   (APIENTRY *PFNGLGETPROGRAMIVPROC)(GLuint program, GLenum pname, GLint *params);
typedef void   (APIENTRY *PFNGLGETPROGRAMINFOLOGPROC)(GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog);

/* Uniforms */
typedef GLint  (APIENTRY *PFNGLGETUNIFORMLOCATIONPROC)(GLuint program, const GLchar *name);
typedef void   (APIENTRY *PFNGLUNIFORM1IPROC)(GLint location, GLint v0);
typedef void   (APIENTRY *PFNGLUNIFORM1FPROC)(GLint location, GLfloat v0);
typedef void   (APIENTRY *PFNGLUNIFORM3FPROC)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
typedef void   (APIENTRY *PFNGLUNIFORM3FVPROC)(GLint location, GLsizei count, const GLfloat *value);
typedef void   (APIENTRY *PFNGLUNIFORMMATRIX4FVPROC)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);

/* Textures */
typedef void (APIENTRY *PFNGLACTIVETEXTUREPROC)(GLenum texture);
typedef void (APIENTRY *PFNGLGENERATEMIPMAPPROC)(GLenum target);

/* Drawing */
typedef void (APIENTRY *PFNGLDRAWARRAYSINSTANCEDPROC)(GLenum mode, GLint first, GLsizei count, GLsizei instancecount);
typedef void (APIENTRY *PFNGLPOLYGONMODEPROC)(GLenum face, GLenum mode);

/* ---- Function declarations (loaded at runtime) ---- */

extern PFNGLGENVERTEXARRAYSPROC       glad_glGenVertexArrays;
extern PFNGLBINDVERTEXARRAYPROC       glad_glBindVertexArray;
extern PFNGLDELETEVERTEXARRAYSPROC    glad_glDeleteVertexArrays;
extern PFNGLGENBUFFERSPROC            glad_glGenBuffers;
extern PFNGLBINDBUFFERPROC            glad_glBindBuffer;
extern PFNGLBUFFERDATAPROC            glad_glBufferData;
extern PFNGLDELETEBUFFERSPROC         glad_glDeleteBuffers;
extern PFNGLENABLEVERTEXATTRIBARRAYPROC glad_glEnableVertexAttribArray;
extern PFNGLVERTEXATTRIBPOINTERPROC  glad_glVertexAttribPointer;
extern PFNGLVERTEXATTRIBDIVISORPROC  glad_glVertexAttribDivisor;

extern PFNGLCREATESHADERPROC          glad_glCreateShader;
extern PFNGLSHADERSOURCEPROC          glad_glShaderSource;
extern PFNGLCOMPILESHADERPROC         glad_glCompileShader;
extern PFNGLDELETESHADERPROC          glad_glDeleteShader;
extern PFNGLGETSHADERIVPROC           glad_glGetShaderiv;
extern PFNGLGETSHADERINFOLOGPROC      glad_glGetShaderInfoLog;

extern PFNGLCREATEPROGRAMPROC         glad_glCreateProgram;
extern PFNGLATTACHSHADERPROC          glad_glAttachShader;
extern PFNGLLINKPROGRAMPROC           glad_glLinkProgram;
extern PFNGLUSEPROGRAMPROC            glad_glUseProgram;
extern PFNGLDELETEPROGRAMPROC         glad_glDeleteProgram;
extern PFNGLGETPROGRAMIVPROC          glad_glGetProgramiv;
extern PFNGLGETPROGRAMINFOLOGPROC     glad_glGetProgramInfoLog;

extern PFNGLGETUNIFORMLOCATIONPROC    glad_glGetUniformLocation;
extern PFNGLUNIFORM1IPROC             glad_glUniform1i;
extern PFNGLUNIFORM1FPROC             glad_glUniform1f;
extern PFNGLUNIFORM3FPROC             glad_glUniform3f;
extern PFNGLUNIFORM3FVPROC            glad_glUniform3fv;
extern PFNGLUNIFORMMATRIX4FVPROC     glad_glUniformMatrix4fv;

extern PFNGLACTIVETEXTUREPROC         glad_glActiveTexture;
extern PFNGLGENERATEMIPMAPPROC        glad_glGenerateMipmap;

extern PFNGLDRAWARRAYSINSTANCEDPROC  glad_glDrawArraysInstanced;
extern PFNGLPOLYGONMODEPROC           glad_glPolygonMode;

/* ---- Convenience macros (transparent replacement) ---- */

#define glGenVertexArrays       glad_glGenVertexArrays
#define glBindVertexArray       glad_glBindVertexArray
#define glDeleteVertexArrays    glad_glDeleteVertexArrays
#define glGenBuffers            glad_glGenBuffers
#define glBindBuffer            glad_glBindBuffer
#define glBufferData            glad_glBufferData
#define glDeleteBuffers         glad_glDeleteBuffers
#define glEnableVertexAttribArray glad_glEnableVertexAttribArray
#define glVertexAttribPointer   glad_glVertexAttribPointer
#define glVertexAttribDivisor   glad_glVertexAttribDivisor
#define glCreateShader          glad_glCreateShader
#define glShaderSource          glad_glShaderSource
#define glCompileShader         glad_glCompileShader
#define glDeleteShader          glad_glDeleteShader
#define glGetShaderiv           glad_glGetShaderiv
#define glGetShaderInfoLog      glad_glGetShaderInfoLog
#define glCreateProgram         glad_glCreateProgram
#define glAttachShader          glad_glAttachShader
#define glLinkProgram           glad_glLinkProgram
#define glUseProgram            glad_glUseProgram
#define glDeleteProgram         glad_glDeleteProgram
#define glGetProgramiv          glad_glGetProgramiv
#define glGetProgramInfoLog     glad_glGetProgramInfoLog
#define glGetUniformLocation    glad_glGetUniformLocation
#define glUniform1i             glad_glUniform1i
#define glUniform1f             glad_glUniform1f
#define glUniform3f             glad_glUniform3f
#define glUniform3fv            glad_glUniform3fv
#define glUniformMatrix4fv      glad_glUniformMatrix4fv
#define glActiveTexture         glad_glActiveTexture
#define glGenerateMipmap        glad_glGenerateMipmap
#define glDrawArraysInstanced   glad_glDrawArraysInstanced
#define glPolygonMode           glad_glPolygonMode

/* GL constants not in gl.h (OpenGL 1.1) */
#define GL_ARRAY_BUFFER                   0x8892
#define GL_ELEMENT_ARRAY_BUFFER           0x8893
#define GL_STATIC_DRAW                    0x88E4
#define GL_DYNAMIC_DRAW                   0x88E8
#define GL_FRAGMENT_SHADER                0x8B30
#define GL_VERTEX_SHADER                  0x8B31
#define GL_COMPILE_STATUS                 0x8B81
#define GL_LINK_STATUS                    0x8B82
#define GL_INFO_LOG_LENGTH                0x8B84
#define GL_TEXTURE0                       0x84C0
#define GL_CLAMP_TO_EDGE                  0x812F
#define GL_MULTISAMPLE                    0x809D
#define GL_LINE                           0x1B01
#define GL_FILL                           0x1B02

/* ---- Loader function ---- */
typedef void* (*GLADloadproc)(const char *name);
int gladLoadGLLoader(GLADloadproc load);

#ifdef __cplusplus
}
#endif

#endif /* GLAD_H */
