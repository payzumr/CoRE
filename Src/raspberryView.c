/**
 * Autor: Orhan Aykac
 * Projekt: Core
 * Oberflaeche zum Anzeigen der
 * Kamera Bilder vom VW
 * 
 * 
 * */

#include "raspberryView.h"
#define BILD_1 0
#define BILD_2 1

static int number = 0;

typedef struct _widgetStruct {
    GtkWidget *window;
    GtkWidget *event_box;
    GtkWidget *image;
} MyWidgets;

MyWidgets myWidgets;
GThread   *thread;
GError    *error = NULL;
GdkPixbuf * pixbuf;	


static gpointer thread_func( gpointer data )
{
	recv_func(data);
	return NULL;
} 

void event_window_destroy(GtkWidget *object, gpointer data) {
    gtk_main_quit();
}


void event_box_button_press(GtkWidget *widget,
   GdkEventButton *event,
   GtkImage  *image) {

  g_print("Event box clicked\n");
  
  
  switch(number){
    case BILD_1:
	
	pixbuf = gdk_pixbuf_new_from_file_at_scale   ("auto3.jpg",
                                                         320,
                                                         240,
                                                         FALSE,
                                                         &error);
														 
	gtk_image_set_from_pixbuf (image, pixbuf);
	//gtk_image_set_from_file(image,"auto3.jpg");
	number = 1;
      break;
    case BILD_2:
	pixbuf = gdk_pixbuf_new_from_file_at_scale   ("auto1.jpg",
                                                         320,
                                                         240,
                                                         FALSE,
                                                         &error);
														 
	gtk_image_set_from_pixbuf (image, pixbuf);
	//gtk_image_set_from_file(image,"auto1.jpg");
	number = 0;
      break;
    default: g_print("Sorry no image \n");
    
  }
    
}


gboolean update_function(GdkPixbuf  *pix) {
    //g_print("Update Function called\n");
	
	//gtk_image_set_from_file(image,"output.jpg");
	
	// pixbuf = gdk_pixbuf_new_from_file_at_scale   ("output.jpg",
                                                         // 320,
                                                         // 240,
                                                         // FALSE,
                                                         // &error);
	


	
	
	//pixbuf = gdk_pixbuf_loader_get_pixbuf (loader);
	gtk_image_set_from_pixbuf(GTK_IMAGE(myWidgets.image), pix );
	g_object_unref(pix);
	
    return FALSE;
}


void init_widgets(MyWidgets *wi) {
  
    wi->window = gtk_window_new(GTK_WINDOW_TOPLEVEL); 
	gtk_window_set_default_size(GTK_WINDOW(wi->window),320,240);
    gtk_window_set_resizable(GTK_WINDOW(wi->window),FALSE);
    
    wi->image = gtk_image_new_from_file("norecv.jpg");

    wi->event_box = gtk_event_box_new();
    gtk_widget_set_size_request(wi->event_box,320,240);
//	gtk_window_fullscreen(GTK_WINDOW(wi->window));
}


void create_signals(MyWidgets *wi) {

    g_signal_connect(wi->window,
       "destroy",
       G_CALLBACK(event_window_destroy),
       NULL);

		g_signal_connect(wi->event_box,
       "button-press-event",
       G_CALLBACK(event_box_button_press),
       GTK_IMAGE(wi-> image ));
}


int main(int argc, char *argv[]) {

    gtk_init(&argc, &argv);

    init_widgets(&myWidgets);
    gtk_container_add(GTK_CONTAINER(myWidgets.event_box), myWidgets.image);
   
    gtk_container_add(GTK_CONTAINER(myWidgets.window), myWidgets.event_box);

    create_signals(&myWidgets);

	thread = g_thread_new("recv", thread_func, myWidgets.image);
	
		if( ! thread )
    {
#ifdef DEBUG_MSG
        g_print( "Error: %s\n", error->message );
#endif
        return( -1 );
    }
	
	gtk_widget_show_all(myWidgets.window);
	
    gtk_main();

  	g_print(":::Ende der Applikation::: \n");

    return 0;
} 
