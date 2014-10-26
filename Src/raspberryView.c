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
    case BILD_1: gtk_image_set_from_file(image,"auto2.jpg"); number = 1;
      break;
    case BILD_2: gtk_image_set_from_file(image,"auto1.jpg"); number = 0;
      break;
    default: g_print("Sorry no image \n");
    
  }
    
}

void event_image_reload(GtkImage *image,gchar *f){
  gtk_image_set_from_file(image,f);
}

void init_widgets(MyWidgets *wi) {
  
    wi->window = gtk_window_new(GTK_WINDOW_TOPLEVEL); 
    gtk_window_set_title(GTK_WINDOW(wi->window),"Kamera"); 
    gtk_window_set_resizable(GTK_WINDOW(wi->window),FALSE);
    gtk_window_set_position(GTK_WINDOW(wi->window),GTK_WIN_POS_CENTER);
    gtk_widget_set_size_request(wi->window,320,240);
    
    wi->image = gtk_image_new_from_file("norecv.jpg");

    wi->event_box = gtk_event_box_new();
    gtk_widget_set_size_request(wi->event_box,320,240);
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

	// /* Secure glib */
     // if( ! g_thread_supported() )
         // g_thread_init( NULL );

	/* Secure gtk */
    gdk_threads_init();

    /* Obtain gtk's global lock */
    gdk_threads_enter();

    gtk_init(&argc, &argv);

    init_widgets(&myWidgets);
    gtk_container_add(GTK_CONTAINER(myWidgets.event_box), myWidgets.image);
   
    gtk_container_add(GTK_CONTAINER(myWidgets.window), myWidgets.event_box);

    create_signals(&myWidgets);

    
	
	/* Create new thread */
    //thread = g_thread_create( thread_func, (gpointer)image, FALSE, &error );
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
	
	/* Release gtk's global lock */
  	gdk_threads_leave();

  	g_print(":::Ende der Applikation::: \n");

    return 0;
} 
