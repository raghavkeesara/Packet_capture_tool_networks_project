#include <gtk/gtk.h>

GtkBuilder      *builder; 
GtkWidget       *window;


int main(int argc, char *argv[])
{
 
    gtk_init(&argc, &argv);
 
    builder = gtk_builder_new();
    gtk_builder_add_from_file (builder, "Unsaved.glade", NULL);
 
    window = GTK_WIDGET(gtk_builder_get_object(builder, "window_main"));
    gtk_builder_connect_signals(builder, NULL);
 
    g_object_unref(builder);
 
    gtk_widget_show(window);                
    gtk_main();
 
    return 0;
}

void open_clicked_cb() 
{
   FILE *fp;
   //char str[] = "This is tutorialspoint.com";

   fp = fopen( "file2.txt" , "w" );
   fprintf(fp ,"hoo");

   fclose(fp);
   //printf("hello!");
    
}

void refresh_clicked_cb()
{

}

void Stop_clicked_cb() 
{

}
 
void on_window_main_destroy()
{
    gtk_main_quit();
}
