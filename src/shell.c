/**
 * @file      shell.c
 * @author    vincent penne <ziggy@sashipa.com>
 * @date      2002/08/11
 * @brief     shell support for dcplaya
 * @version   $Id: shell.c,v 1.7 2002-09-16 05:25:08 zig Exp $
 */

#include <kos.h>

#include <ctype.h>

#include "shell.h"
#include "console.h"

#include "lef.h"

static char input[256];
static int input_pos;

#define CONSOLE_Y 50
#define CONSOLE_OUT_Y -600
static int show_console = 0;
static float console_y = CONSOLE_OUT_Y;

lef_prog_t * shell_lef;
shell_shutdown_func_t shell_lef_shutdown_func;

char * shell_lef_fname = "/pc" DREAMMP3_HOME "dynshell/dynshell.lef";

static shell_command_func_t shell_command_func;



#define MAX_COMMANDS 128

static char * history[MAX_COMMANDS];
static int write_history;
static int read_history;

static char * commands[MAX_COMMANDS];
static int write_command;
static int read_command;





static void shell_update_keyboard()
{

  int key;

  key = csl_peekchar();

/*  if (key != -1)
    printf(" --> %d\n", key);*/
  
  switch (key) {
  case -1:
    return;

  case KBD_KEY_F1<<8: // F1
    shell_load(shell_lef_fname);
    break;

  case KBD_KEY_F2<<8: // F2
    csl_putstring(csl_main_console, "dofile(home..[[autorun.lua]])\n");
    shell_command("dofile(home..[[autorun.lua]])");
    break;

  case 8:
    if (input_pos > 0) {
      csl_putchar(csl_main_console, key);
      input_pos--;
    }
    break;

  case 13:
    csl_putchar(csl_main_console, '\n');

    input[input_pos] = 0;
    shell_command(input);

    input_pos = 0;

    break;

  case '`':
    show_console = !show_console;
    break;

  default:
    if (key == ' ' || isprint(key)) {
      csl_putchar(csl_main_console, key);
      input[input_pos++] = key;
    }
    break;
  }

}

static spinlock_t shellmutex;

static void lockshell(void)
{
  spinlock_lock(&shellmutex);
}

void unlockshell(void)
{
  spinlock_unlock(&shellmutex);
}



void shell_load(const char * fname)
{
  int fd;

  /* better to make sure all previous commands are finished */
  shell_wait();

  lockshell();

  /* Shutdown previously opened shell */
  if (shell_lef) {
    if (shell_lef_shutdown_func) {
      shell_lef_shutdown_func();
      shell_lef_shutdown_func = 0;
    }
    lef_free(shell_lef);
    shell_lef = 0;
  }


  /* Load and launch new shell */
  fd = fs_open(fname, O_RDONLY);
  if (fd) {
    
    /* $$$ After this call, fd is closed by lef_load !!! */
    shell_lef = lef_load(fd);
    fd = 0;
  }

  if (shell_lef) {
    shell_lef_shutdown_func = (shell_shutdown_func_t) shell_lef->main(0, 0);
  }

  unlockshell();

}



static void shell_thread(void * param)
{
  for (;;) {

    while (write_command == read_command) {
      thd_pass();

      /* Keyboard update */
      shell_update_keyboard();
    }

    if (shell_command_func) {
      lockshell();
      shell_command_func(commands[read_command]);
      unlockshell();
    } else
      printf("shell: don't know what to do with command '%s' (no shell loaded !)\n", commands[read_command]);

    free(commands[read_command]);
    read_command = (read_command+1)%MAX_COMMANDS;

 }
}



int shell_init()
{

  thd_create(shell_thread, 0);

  shell_load(shell_lef_fname);
  
  return 0;
}

void shell_shutdown()
{
#warning "TODO"
}

void shell_update(float frameTime)
{

  /* Console movement handling */
  if (show_console || read_command != write_command) {
    console_y += 5 * frameTime * (CONSOLE_Y - console_y);
  } else {
    console_y += 5 * frameTime * (CONSOLE_OUT_Y - console_y);
  }

  if (console_y < CONSOLE_OUT_Y + 5)
    csl_disable_render_mode(csl_main_console, CSL_RENDER_WINDOW);
  else
    csl_enable_render_mode(csl_main_console, CSL_RENDER_WINDOW);
      
  csl_main_console->window.y = console_y;
}


static spinlock_t commandmutex;

static void lockcommand(void)
{
  spinlock_lock(&commandmutex);
}

void unlockcommand(void)
{
  spinlock_unlock(&commandmutex);
}


int shell_command(const char * com)
{
  //printf("COMMAND <%s>\n", com);

  lockcommand();

  commands[write_command] = strdup(com);
  write_command = (write_command+1) % MAX_COMMANDS;

  unlockcommand();

  return 0;

}

void shell_wait()
{
  while (write_command != read_command)
    thd_pass();
}

shell_command_func_t shell_set_command_func(shell_command_func_t func)
{
  shell_command_func_t old = shell_command_func;

  shell_command_func = func;

  return old;
}