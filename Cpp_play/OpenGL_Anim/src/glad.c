/*
 * Minimal OpenGL 3.3 Core loader implementation
 */
#include "glad.h"
#include <stdio.h>

PFNGLGENVERTEXARRAYSPROC       glad_glGenVertexArrays = NULL;
PFNGLBINDVERTEXARRAYPROC       glad_glBindVertexArray = NULL;
PFNGLDELETEVERTEXARRAYSPROC    glad_glDeleteVertexArrays = NULL;
PFNGLGENBUFFERSPROC            glad_glGenBuffers = NULL;
PFNGLBINDBUFFERPROC            glad_glBindBuffer = NULL;
PFNGLBUFFERDATAPROC            glad_glBufferData = NULL;
PFNGLDELETEBUFFERSPROC         glad_glDeleteBuffers = NULL;
PFNGLENABLEVERTEXATTRIBARRAYPROC glad_glEnableVertexAttribArray = NULL;
PFNGLVERTEXATTRIBPOINTERPROC  glad_glVertexAttribPointer = NULL;
PFNGLVERTEXATTRIBDIVISORPROC  glad_glVertexAttribDivisor = NULL;

PFNGLCREATESHADERPROC          glad_glCreateShader = NULL;
PFNGLSHADERSOURCEPROC          glad_glShaderSource = NULL;
PFNGLCOMPILESHADERPROC         glad_glCompileShader = NULL;
PFNGLDELETESHADERPROC          glad_glDeleteShader = NULL;
PFNGLGETSHADERIVPROC           glad_glGetShaderiv = NULL;
PFNGLGETSHADERINFOLOGPROC      glad_glGetShaderInfoLog = NULL;

PFNGLCREATEPROGRAMPROC         glad_glCreateProgram = NULL;
PFNGLATTACHSHADERPROC          glad_glAttachShader = NULL;
PFNGLLINKPROGRAMPROC           glad_glLinkProgram = NULL;
PFNGLUSEPROGRAMPROC            glad_glUseProgram = NULL;
PFNGLDELETEPROGRAMPROC         glad_glDeleteProgram = NULL;
PFNGLGETPROGRAMIVPROC          glad_glGetProgramiv = NULL;
PFNGLGETPROGRAMINFOLOGPROC     glad_glGetProgramInfoLog = NULL;

PFNGLGETUNIFORMLOCATIONPROC    glad_glGetUniformLocation = NULL;
PFNGLUNIFORM1IPROC             glad_glUniform1i = NULL;
PFNGLUNIFORM1FPROC             glad_glUniform1f = NULL;
PFNGLUNIFORM3FPROC             glad_glUniform3f = NULL;
PFNGLUNIFORM3FVPROC            glad_glUniform3fv = NULL;
PFNGLUNIFORMMATRIX4FVPROC     glad_glUniformMatrix4fv = NULL;

PFNGLACTIVETEXTUREPROC         glad_glActiveTexture = NULL;
PFNGLGENERATEMIPMAPPROC        glad_glGenerateMipmap = NULL;

PFNGLDRAWARRAYSINSTANCEDPROC  glad_glDrawArraysInstanced = NULL;
PFNGLPOLYGONMODEPROC           glad_glPolygonMode = NULL;

static void load_gl_funcs(GLADloadproc load) {
    glad_glGenVertexArrays       = (PFNGLGENVERTEXARRAYSPROC)    load("glGenVertexArrays");
    glad_glBindVertexArray       = (PFNGLBINDVERTEXARRAYPROC)    load("glBindVertexArray");
    glad_glDeleteVertexArrays    = (PFNGLDELETEVERTEXARRAYSPROC) load("glDeleteVertexArrays");
    glad_glGenBuffers            = (PFNGLGENBUFFERSPROC)         load("glGenBuffers");
    glad_glBindBuffer            = (PFNGLBINDBUFFERPROC)         load("glBindBuffer");
    glad_glBufferData            = (PFNGLBUFFERDATAPROC)         load("glBufferData");
    glad_glDeleteBuffers         = (PFNGLDELETEBUFFERSPROC)      load("glDeleteBuffers");
    glad_glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC) load("glEnableVertexAttribArray");
    glad_glVertexAttribPointer   = (PFNGLVERTEXATTRIBPOINTERPROC) load("glVertexAttribPointer");
    glad_glVertexAttribDivisor   = (PFNGLVERTEXATTRIBDIVISORPROC) load("glVertexAttribDivisor");

    glad_glCreateShader          = (PFNGLCREATESHADERPROC)       load("glCreateShader");
    glad_glShaderSource          = (PFNGLSHADERSOURCEPROC)       load("glShaderSource");
    glad_glCompileShader         = (PFNGLCOMPILESHADERPROC)      load("glCompileShader");
    glad_glDeleteShader          = (PFNGLDELETESHADERPROC)       load("glDeleteShader");
    glad_glGetShaderiv           = (PFNGLGETSHADERIVPROC)        load("glGetShaderiv");
    glad_glGetShaderInfoLog      = (PFNGLGETSHADERINFOLOGPROC)   load("glGetShaderInfoLog");

    glad_glCreateProgram         = (PFNGLCREATEPROGRAMPROC)      load("glCreateProgram");
    glad_glAttachShader          = (PFNGLATTACHSHADERPROC)       load("glAttachShader");
    glad_glLinkProgram           = (PFNGLLINKPROGRAMPROC)        load("glLinkProgram");
    glad_glUseProgram            = (PFNGLUSEPROGRAMPROC)         load("glUseProgram");
    glad_glDeleteProgram         = (PFNGLDELETEPROGRAMPROC)      load("glDeleteProgram");
    glad_glGetProgramiv          = (PFNGLGETPROGRAMIVPROC)       load("glGetProgramiv");
    glad_glGetProgramInfoLog     = (PFNGLGETPROGRAMINFOLOGPROC)  load("glGetProgramInfoLog");

    glad_glGetUniformLocation    = (PFNGLGETUNIFORMLOCATIONPROC) load("glGetUniformLocation");
    glad_glUniform1i             = (PFNGLUNIFORM1IPROC)          load("glUniform1i");
    glad_glUniform1f             = (PFNGLUNIFORM1FPROC)          load("glUniform1f");
    glad_glUniform3f             = (PFNGLUNIFORM3FPROC)          load("glUniform3f");
    glad_glUniform3fv            = (PFNGLUNIFORM3FVPROC)         load("glUniform3fv");
    glad_glUniformMatrix4fv     = (PFNGLUNIFORMMATRIX4FVPROC)  load("glUniformMatrix4fv");

    glad_glActiveTexture         = (PFNGLACTIVETEXTUREPROC)      load("glActiveTexture");
    glad_glGenerateMipmap        = (PFNGLGENERATEMIPMAPPROC)     load("glGenerateMipmap");

    glad_glDrawArraysInstanced  = (PFNGLDRAWARRAYSINSTANCEDPROC) load("glDrawArraysInstanced");
    glad_glPolygonMode           = (PFNGLPOLYGONMODEPROC)        load("glPolygonMode");
}

int gladLoadGLLoader(GLADloadproc load) {
    if (load == NULL) return 0;
    load_gl_funcs(load);

    /* Check that critical functions loaded */
    if (!glad_glCreateShader || !glad_glCreateProgram || !glad_glGenVertexArrays) {
        fprintf(stderr, "GLAD: Failed to load critical OpenGL functions\n");
        return 0;
    }
    return 1;
}
