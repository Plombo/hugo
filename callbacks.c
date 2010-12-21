#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <dirent.h>
#include <gtk/gtk.h>

#include "callbacks.h"
#include "interface.h"
#include "support.h"

#include "gtk_main.h"
#include "pce.h"
#include "iniconfig.h"

#ifdef SDL
#include "SDL.h"
#endif

static char tmp_buf[100];


void
on_open1_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  get_directory_from_filename(initial_path);
  gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(fileselector_window), initial_path);
  gtk_widget_show(fileselector_window);
}


void
on_save1_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    savegame();
}


void
on_load1_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	loadgame();
}

void
on_pause_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	is_paused = !is_paused;
	
#if 0
	SDL_Event sdlev;
	sdlev.type = SDL_KEYDOWN;
	sdlev.key.state = SDL_PRESSED;
	sdlev.key.keysym.sym = SDLK_PAUSE;
	SDL_PushEvent(&sdlev);
#endif
}


void
on_framestep_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	framestep_requested = 1;
}


void
on_input_setting_1_activate            (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  gtk_widget_show(input_settings_window);
}


void
on_hugo_manual1_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
#ifdef OLD_CRAPPY_MANUAL
	// use text manual from original Hu-Go!
	gtk_widget_show(manual_window);
#else
	// Open the HTML manual in a browser. Adapted from GensUI::LaunchBrowser() 
	// in Gens/GS.
	char cwd[1024], url[1024];
	static const char xdg_open[] = "/usr/bin/xdg-open";
	
	// Get the current working directory and open the url at ./doc/index.html.
	if(!getcwd(cwd, 1024))
	{
		fprintf(stderr, "Unable to get current working directory\n");
		return;
	}
	sprintf(url, "file:///%s/%s", cwd, "doc/index.html");
	
	// Use xdg-open.
	if (access(xdg_open, X_OK) != 0)
	{
		if (access(xdg_open, F_OK) != 0)
		{
			fprintf(stderr, "%s not found.\n", xdg_open);
		}
		else
		{
			fprintf(stderr, "Cannot execute %s .\n", xdg_open);
		}
		
		return;
	}
	
	pid_t pID = vfork();
	if (pID < 0)
	{
		fprintf(stderr, "vfork() failed: %d (%s)\n", errno, strerror(errno));
		return;
	}
	else if (pID == 0)
	{
		// Child process. Run xdg-open.
		execl(xdg_open, xdg_open, url, NULL);
	}
	else
	{
		fprintf(stderr, "pID > 0.  Not sure how to handle this.\n");
	}
#endif
}



void
on_about_1_activate                    (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	gtk_widget_show(about_window);
}


void
on_mainWindow_destroy                  (GtkObject       *object,
                                        gpointer         user_data)
{
	gtk_main_quit();
}


void
on_ok_button1_clicked                  (GtkButton       *button,
                                        gpointer         user_data)
{	
	char* filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(fileselector_window));
	DIR* dir = opendir(filename);
	
	if(dir != NULL) // selected file is a directory; change the directory
	{
		closedir(dir);
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(fileselector_window), filename);
	}
	else // normal file selected; open the file and start the emulator
	{
		gtk_widget_hide(fileselector_window);
		CD_emulation = 0;
		strcpy( cart_name, gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(fileselector_window)));

		/*
		 * We need to flush any gtk events waiting to happen (like the widget hide
		     * from above) to avoid a deadlock when starting a game fullscreen (at least
		     * in linux).
		     */
		while (gtk_events_pending())
		  gtk_main_iteration();

		play_game();
	}
}


void
on_button1_clicked                     (GtkButton       *button,
                                        gpointer         user_data)
{
  on_open1_activate(NULL, NULL);
}


void
on_cancel_button1_clicked              (GtkButton       *button,
                                        gpointer         user_data)
{
	gtk_widget_hide(fileselector_window);
}


void
on_button_close_input_settings_window_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data)
{
	gtk_widget_hide(input_settings_window);
}


void
on_button2_clicked                     (GtkButton       *button,
                                        gpointer         user_data)
{
  if (NULL == search_possible_syscard())
    {
      GtkWidget* dialog;
      dialog = gtk_message_dialog_new (GTK_WINDOW (main_window),
				       GTK_DIALOG_DESTROY_WITH_PARENT,
				       GTK_MESSAGE_ERROR,
				       GTK_BUTTONS_OK,
				       "CD system card not found !\nDid you set \"CD system filename\" ?");
      gtk_dialog_run (GTK_DIALOG (dialog));
      gtk_widget_destroy (dialog);
    }
  else
    {
      CD_emulation = 1;
      play_game();
    }
}


void
on_general_settings_1_activate         (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	gtk_widget_show(general_settings_window);
}


void
on_button_close_about_clicked          (GtkButton       *button,
                                        gpointer         user_data)
{
	gtk_widget_hide(about_window);
}


void
on_button_browse_cd_system_clicked     (GtkButton       *button,
                                        gpointer         user_data)
{
	gtk_widget_show(fileselector_cd_system);
}


void
on_buttongeneral_config_close_clicked  (GtkButton       *button,
                                        gpointer         user_data)
{
	gtk_widget_hide(general_settings_window);
	gtk_general_settings_grab();
}


void
on_button_general_config_save_clicked  (GtkButton       *button,
                                        gpointer         user_data)
{
	gtk_general_settings_grab();
	save_config();
}


void
on_button_general_config_cancel_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data)
{
	gtk_widget_hide(general_settings_window);
}


void
on_general_settings_window_show        (GtkWidget       *widget,
                                        gpointer         user_data)
{
	gtk_general_settings_set();
}


void
on_ok_button_cd_system_clicked         (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkEntry* temp_entry;

	gtk_widget_hide(fileselector_cd_system);
	strncpy (cdsystem_path, gtk_file_selection_get_filename((GtkFileSelection*)fileselector_cd_system), PATH_MAX);
	temp_entry = (GtkEntry*)lookup_widget(general_settings_window, "entry_cd_system_filename");
	gtk_entry_set_text(temp_entry, cdsystem_path);	
}


void
on_cancel_button_cd_system_clicked     (GtkButton       *button,
                                        gpointer         user_data)
{
	gtk_widget_hide(fileselector_cd_system);
}


void
on_ok_button_cd_path_clicked           (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkEntry* temp_entry;	
	
	gtk_widget_hide(fileselector_cd_path);
	strcpy (ISO_filename, gtk_file_selection_get_filename((GtkFileSelection*)fileselector_cd_path));
	temp_entry = (GtkEntry*)lookup_widget(general_settings_window, "entry_cd_path");
	gtk_entry_set_text(temp_entry, ISO_filename);

}


void
on_cancel_button_cd_path_clicked       (GtkButton       *button,
                                        gpointer         user_data)
{
	gtk_widget_hide(fileselector_cd_path);
}

void
on_button_browse_rom_dirname_clicked   (GtkButton       *button,
                                        gpointer         user_data)
{
	gtk_widget_show(fileselector_rom_path);
}


void
on_button_browse_cd_path_clicked       (GtkButton       *button,
                                        gpointer         user_data)
{
	gtk_widget_show(fileselector_cd_path);
}


void
on_ok_button_rom_path_clicked          (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkEntry* temp_entry;
	
	gtk_widget_hide(fileselector_rom_path);
	strcpy (initial_path, gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(fileselector_rom_path)));
	
	get_directory_from_filename(initial_path);
	
//	if (strrchr(initial_path, '/') != NULL)
//		*strrchr(initial_path, '/') = 0;
	temp_entry = (GtkEntry*)lookup_widget(general_settings_window, "entry_rom_basedir");
	gtk_entry_set_text(temp_entry, initial_path);
}


void
on_cancel_button_rom_path_clicked      (GtkButton       *button,
                                        gpointer         user_data)
{
	gtk_widget_hide(fileselector_rom_path);
}


void
on_button_manual_close_clicked         (GtkButton       *button,
                                        gpointer         user_data)
{
	gtk_widget_hide(manual_window);
}

//void
//on_input_settings_window_show          (GtkWidget       *widget,
//                                        gpointer         user_data)
//{
//	gtk_show_config(0);
//}


void
on_open_cd1_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{

}


void
on_save_settings1_activate             (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	save_config();
}

void
on_option_config_number_changed        (GtkOptionMenu   *optionmenu,
                                        gpointer         user_data)
{

}


void
on_button_input_configuration_clicked  (GtkButton       *button,
                                        gpointer         user_data)
{
	
	char* button_name = (char*) user_data;
	int player_number = button_name[strlen(button_name) - 1] - '0';
	int direction_index = 0;
		
	if ((player_number < 0) || (player_number > 4))
	{
		Log("Abnormal player_number in %s at %s:%s\nAborting\n",
			__FUNCTION__,
			__FILE__,
			__LINE__);
		return;
	}
	
	for (; direction_index < J_MAX; direction_index ++)
	{
		if (!strncmp(joymap_reverse[direction_index],
				button_name,
				strlen(button_name) - 1))
			{
				break;	
			}
	} 

	gtk_grab_control(direction_index, player_number);

}

void
on_spinbutton_configuration_value_changed
                                        (GtkSpinButton   *spinbutton,
                                        gpointer         user_data)
{
	int index = gtk_spin_button_get_value_as_int((GtkSpinButton*) spinbutton);
	set_gui_configuration_index(index);
}


void
on_window_input_settings_show          (GtkWidget       *widget,
                                        gpointer         user_data)
{
	GtkSpinButton * spinbutton = (GtkSpinButton*)lookup_widget(widget, "spinbutton_configuration");
	on_spinbutton_configuration_value_changed(spinbutton, (gpointer)NULL);
	gtk_copy_current_configuration();
}


gboolean
on_window_input_settings_delete_event  (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
	gtk_widget_hide(input_settings_window);
	return TRUE;
}


void
on_button_input_ok_activate            (GtkButton       *button,
                                        gpointer         user_data)
{
	gtk_confirm_configuration();
	gtk_widget_hide(input_settings_window);
}


void
on_button_input_cancel_activate        (GtkButton       *button,
                                        gpointer         user_data)
{
	gtk_widget_hide(input_settings_window);
}

void
on_spinbutton_joydev_value_changed     (GtkSpinButton   *spinbutton,
                                        gpointer         user_data)
{
	GtkSpinButton* temp_spin_button = NULL;
	int i = 0;
	int player_number = -1;
	
	for (i = 0; i < 5; i++)
	{
		sprintf(tmp_buf, "spinbutton_joydev%d", i);
		if (spinbutton == (GtkSpinButton*)lookup_widget(input_settings_window, tmp_buf))
		{
			player_number = i;
			break;
		}
	}

	printf("Checked that joydev %d changed\n", player_number);
	
	if (player_number != -1)
	{
		printf("Read value %d\n", gtk_spin_button_get_value_as_int(spinbutton));
		set_gui_joydev(player_number, gtk_spin_button_get_value_as_int(spinbutton));
	}
	
	gtk_update_configuration(FALSE);
}


gboolean
on_general_settings_window_delete_event
                                        (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
	gtk_widget_hide(general_settings_window);
  return TRUE;
}


gboolean
on_fileselection_cd_system_delete_event
                                        (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
	gtk_widget_hide(fileselector_cd_system);
  return TRUE;
}


gboolean
on_fileselection_cd_path_delete_event  (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
	gtk_widget_hide(fileselector_cd_path);
  return TRUE;
}


gboolean
on_fileselection_rom_path_delete_event (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
	gtk_widget_hide(fileselector_rom_path);
  return TRUE;
}


gboolean
on_fileselection1_delete_event         (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
	gtk_widget_hide(fileselector_window);
  return TRUE;
}


gboolean
on_window_about_delete_event           (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
	gtk_widget_hide(about_window);
  return TRUE;
}

// Function adapted from Gens/GS (source/gens/ui/gtk/gens/gens_window_callbacks.c)
void on_sdlsocket_drag_data_received(GtkWidget *widget, GdkDragContext *context, gint x, gint y,
				    GtkSelectionData *selection_data, guint target_type, guint time,
				    gpointer data)
{
	if (selection_data == NULL || gtk_selection_data_get_length(selection_data) == 0)
	{
		// No selection data.
		gtk_drag_finish(context, FALSE, FALSE, time);
		return;
	}
	
	gboolean dnd_success = FALSE;
	
	gchar *filename = (gchar*)gtk_selection_data_get_data(selection_data);
	if (!filename)
	{
		// Selection data was not text.
		gtk_drag_finish(context, FALSE, FALSE, time);
		return;
	}
	
	if (strncmp(filename, "file:///", 8) == 0)
	{
		// "file:///" prefix. Remove the prefix.
		filename += 7;
	}
	else if (strncmp(filename, "file://", 7) == 0)
	{
		// "file://" prefix. Remove the prefix.
		filename += 6;
	}
	else if (strncmp(filename, "file:/", 6) == 0)
	{
		// "file:/" prefix. Remove the prefix.
		filename += 5;
	}
	else if (strncmp(filename, "desktop:/", 9) == 0)
	{
		// "desktop:/" prefix. Remove the prefix and prepend the user's desktop directory.
		char tmpname[1024] = {""};
		sprintf(tmpname, "%s/Desktop/%s", getenv("HOME"), filename + 9);
		filename = tmpname;
	}
	
	// Unescape the URI.
	char realname[1024];
	int i, j;
	for (i=0, j=0; i<strlen(filename); j++)
	{
		if (filename[i] == '%')
		{
			char tmp[3];
			strncpy(tmp, &filename[i+1], 2);
			realname[j] = (char)strtol(tmp, NULL, 16);
			i += 3;
		} else realname[j] = filename[i++];
		if(realname[j] == '\r' || realname[j] == '\n') realname[j] = 0;
	}
	realname[j] = 0;
	
	// Check that the file actually exists. (TODO)
	gboolean exists = TRUE;
	if (!exists) dnd_success = FALSE;
	
	if (exists)
	{
		CD_emulation = 0;
		strcpy(cart_name, realname);
		/* We need to flush any GTK events waiting to happen to avoid a deadlock
		 * when starting a game fullscreen (at least in Linux). */
		while (gtk_events_pending())
			gtk_main_iteration();
		play_game();
	}
	
	gtk_drag_finish(context, dnd_success, FALSE, time);
}

#ifdef SDL
// Function adapted from Gens/GS (source/gens/input/input_sdl.c)
gint input_sdl_gdk_keysnoop(GtkWidget *grab, GdkEventKey *event, gpointer user_data)
{
	SDL_Event sdlev;
	SDLKey sdlkey;
	int keystate;


	// Only grab keys from the Gens window. (or controller config window)
	if (grab != main_window)
	{
		// Don't push this key onto the SDL event stack.
		return FALSE;
	}

	
	switch (event->type)
	{
		case GDK_KEY_PRESS:
			sdlev.type = SDL_KEYDOWN;
			sdlev.key.state = SDL_PRESSED;
			keystate = 1;
			break;
		
		case GDK_KEY_RELEASE:
			sdlev.type = SDL_KEYUP;
			sdlev.key.state = SDL_RELEASED;
			keystate = 0;
			break;
		
		default:
			fprintf(stderr, "Unhandled GDK event type: %d", event->type);
			return FALSE;
	}
	
	// Convert this keypress from GDK to SDL.
	sdlkey = (SDLKey)input_sdl_gdk_to_gens_keyval(event->keyval);
	
	// Create an SDL event from the keypress.
	sdlev.key.keysym.sym = sdlkey;
	if (sdlkey != 0)
		SDL_PushEvent(&sdlev);
	
	// Change the keyboard state based on the keypress.
	if(key)
		key[sdlkey] = keystate;
	
	// Allow GTK+ to process this key.
	return FALSE;
}
#endif

