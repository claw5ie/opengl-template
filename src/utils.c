typedef GLuint gluint;
typedef GLint glint;
typedef GLenum glenum;

void *
xmalloc(size_t size)
{
  void *data = malloc(size);

  if (data == NULL)
    {
      fprintf(stderr, "ERROR: can't allocate %zu bytes.\n", size);
      abort();
    }

  return data;
}

char *
read_entire_file(const char *filepath, size_t *file_size_loc)
{
  int fd = open(filepath, O_RDONLY);
  if (fd == -1)
    {
      fprintf(stderr, "ERROR: can't open file \'%s\'.\n", filepath);
      abort();
    }

  size_t file_size = 0;

  {
    struct stat stats;
    if (fstat(fd, &stats) == -1)
      abort();

    file_size = stats.st_size;
  }

  char *file_data = xmalloc(file_size + 1);
  if (read(fd, file_data, file_size) != (ssize_t)file_size)
    {
      fprintf(stderr,
              "ERROR: can't read %zu bytes from a file \'%s\'.\n",
              file_size,
              file_data);
      abort();
    }
  file_data[file_size] = '\0';

  close(fd);

  if (file_size_loc != NULL)
    *file_size_loc = file_size;

  return file_data;
}

gluint
create_shader(glenum shader_type, const char *filepath)
{
  assert(shader_type == GL_VERTEX_SHADER
         || shader_type == GL_FRAGMENT_SHADER);

  gluint shader = glCreateShader(shader_type);

  {
    size_t file_size = 0;
    char *file_data = read_entire_file(filepath, &file_size);
    glint line_size = file_size;
    glShaderSource(shader, 1, (void *)&file_data, &line_size);
    free(file_data);
  }

  glint status = 0;
  glCompileShader(shader);
  glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

  if (status != GL_TRUE)
    {
      glint info_log_length = 0;
      glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &info_log_length);

      const char *shader_type_as_string
        = shader_type == GL_VERTEX_SHADER ? "vertex" : "fragment";
      if (info_log_length <= 0)
        fprintf(stderr,
                "ERROR: can't compile %s shader: no error message from OpenGL.\n",
                shader_type_as_string);
      else
        {
          char *error_message = xmalloc(info_log_length + 1);
          glGetShaderInfoLog(shader, info_log_length, NULL, error_message);
          error_message[info_log_length] = '\0';
          fprintf(stderr,
                  "ERROR: can't compile %s shader:\n%s",
                  shader_type_as_string,
                  error_message);
          free(error_message);
        }

      glDeleteShader(shader);
      abort();
    }

  return shader;
}

gluint
create_program(gluint vertex_shader, gluint fragment_shader)
{
  gluint program = glCreateProgram();

  glAttachShader(program, vertex_shader);
  glAttachShader(program, fragment_shader);
  glLinkProgram(program);
  glDetachShader(program, vertex_shader);
  glDetachShader(program, fragment_shader);

  glint status = 0;
  glGetProgramiv(program, GL_LINK_STATUS, &status);

  if (status != GL_TRUE)
    {
      glint info_log_length = 0;
      glGetProgramiv(program, GL_INFO_LOG_LENGTH, &info_log_length);

      if (info_log_length <= 0)
        fputs("ERROR: can't link program: no error message from OpenGL.\n",
              stderr);
      else
        {
          char *error_message = xmalloc(info_log_length + 1);
          glGetProgramInfoLog(program, info_log_length, NULL, error_message);
          error_message[info_log_length] = '\0';
          fprintf(stderr,
                  "ERROR: can't link program:\n%s",
                  error_message);
          free(error_message);
        }

      glDeleteProgram(program);
      abort();
    }

  return program;
}
