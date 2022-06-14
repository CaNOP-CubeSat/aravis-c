#include <arv.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arvgvcpprivate.h>
//#include <aravis-0.8/src/arvgvcpprivate.h>
#include <ImageMagick-7/MagickWand/MagickWand.h>

static gboolean cancel = FALSE;

static void
set_cancel (int signal)
{
	cancel = TRUE;
}

static char *arv_option_camera_name = NULL;
static char *arv_option_debug_domains = NULL;
static gboolean arv_option_auto_buffer = FALSE;
static int arv_option_width = -1;
static int arv_option_height = -1;
static int arv_option_horizontal_binning = -1;
static int arv_option_vertical_binning = -1;

static const GOptionEntry arv_option_entries[] =
{
	{ "name",		'n', 0, G_OPTION_ARG_STRING,
		&arv_option_camera_name,"Camera name", NULL},
	{ "auto",		'a', 0, G_OPTION_ARG_NONE,
		&arv_option_auto_buffer,	"AutoBufferSize", NULL},
	{ "width", 		'w', 0, G_OPTION_ARG_INT,
		&arv_option_width,		"Width", NULL },
	{ "height", 		'h', 0, G_OPTION_ARG_INT,
		&arv_option_height, 		"Height", NULL },
	{ "h-binning", 		'\0', 0, G_OPTION_ARG_INT,
		&arv_option_horizontal_binning,"Horizontal binning", NULL },
	{ "v-binning", 		'\0', 0, G_OPTION_ARG_INT,
		&arv_option_vertical_binning, 	"Vertical binning", NULL },
	{ "debug", 		'd', 0, G_OPTION_ARG_STRING,
		&arv_option_debug_domains, 	"Debug mode", NULL },
	{ NULL }
};

typedef enum {
	ARV_CAMERA_TYPE_BASLER,
	ARV_CAMERA_TYPE_PROSILICA
} ArvCameraType;

//Globals
ArvBuffer *buffer;
size_t strSize;
//unsigned long imgWidth;
//unsigned long imgHeight;

//Functions
void bufferToImage(ArvBuffer* frameBuffer, int imgNum);

int main (int argc, char **argv){
	ArvDevice *device;
	ArvStream *stream;
	GOptionContext *context;
	GError *error = NULL;
	char memory_buffer[100000];
	int i;
	unsigned buffer_count = 0;
	guint64 start_time, time;

	context = g_option_context_new (NULL);
	g_option_context_add_main_entries (context, arv_option_entries, NULL);

	if (!g_option_context_parse (context, &argc, &argv, &error)) {
		g_option_context_free (context);
		g_print ("Option parsing failed: %s\n", error->message);
		g_error_free (error);
		return EXIT_FAILURE;
	}

	g_option_context_free (context);

	arv_debug_enable (arv_option_debug_domains);

	if (arv_option_camera_name == NULL)
		g_print ("Looking for the first available camera\n");
	else
		g_print ("Looking for camera '%s'\n", arv_option_camera_name);

	device = arv_open_device (arv_option_camera_name, NULL);
	if (device != NULL) {
		ArvGc *genicam;
		ArvGcNode *node;
		guint32 value;
		guint32 maximum;
		guint32 minimum;
		guint64 n_processed_buffers;
		guint64 n_failures;
		guint64 n_underruns;
		double v_double;
		double v_double_min;
		double v_double_max;
		const char *v_string;
		gboolean v_boolean;
		int imgNum = 0;
		//ArvBuffer *frame;

		genicam = arv_device_get_genicam (device);

		if (arv_option_width > 0) {
			node = arv_gc_get_node (genicam, "Width");
			arv_gc_integer_set_value (ARV_GC_INTEGER (node), arv_option_width, NULL);
		}
		if (arv_option_height > 0) {
			node = arv_gc_get_node (genicam, "Height");
			arv_gc_integer_set_value (ARV_GC_INTEGER (node), arv_option_height, NULL);
		}
		if (arv_option_horizontal_binning > 0) {
			node = arv_gc_get_node (genicam, "BinningHorizontal");
			arv_gc_integer_set_value (ARV_GC_INTEGER (node), arv_option_horizontal_binning, NULL);
		}
		if (arv_option_vertical_binning > 0) {
			node = arv_gc_get_node (genicam, "BinningVertical");
			arv_gc_integer_set_value (ARV_GC_INTEGER (node), arv_option_vertical_binning, NULL);
		}

		node = arv_gc_get_node (genicam, "DeviceVendorName");
		v_string = arv_gc_string_get_value (ARV_GC_STRING (node), NULL);
		g_print ("vendor        = %s\n", v_string);

		node = arv_gc_get_node (genicam, "DeviceModelName");
		v_string = arv_gc_string_get_value (ARV_GC_STRING (node), NULL);
		g_print ("model         = %s\n", v_string);

		node = arv_gc_get_node (genicam, "DeviceID");
		v_string = arv_gc_string_get_value (ARV_GC_STRING (node), NULL);
		g_print ("device id     = %s\n", v_string);

		node = arv_gc_get_node (genicam, "SensorWidth");
		value = arv_gc_integer_get_value (ARV_GC_INTEGER (node), NULL);
		g_print ("sensor width  = %d\n", value);

		node = arv_gc_get_node (genicam, "SensorHeight");
		value = arv_gc_integer_get_value (ARV_GC_INTEGER (node), NULL);
		g_print ("sensor height = %d\n", value);

		node = arv_gc_get_node (genicam, "Width");
		value = arv_gc_integer_get_value (ARV_GC_INTEGER (node), NULL);
		//imgWidth = (unsigned long)value; // set image width
		maximum = arv_gc_integer_get_max (ARV_GC_INTEGER (node), NULL);
		g_print ("image width   = %d (max:%d)\n", value, maximum);

		node = arv_gc_get_node (genicam, "Height");
		value = arv_gc_integer_get_value (ARV_GC_INTEGER (node), NULL);
		//imgHeight = (unsigned long)value; // set image height
		maximum = arv_gc_integer_get_max (ARV_GC_INTEGER (node), NULL);
		g_print ("image height  = %d (max:%d)\n", value, maximum);

		node = arv_gc_get_node (genicam, "BinningHorizontal");
		if (ARV_IS_GC_NODE (node)) {
			value = arv_gc_integer_get_value (ARV_GC_INTEGER (node), NULL);
			maximum = arv_gc_integer_get_max (ARV_GC_INTEGER (node), NULL);
			minimum = arv_gc_integer_get_min (ARV_GC_INTEGER (node), NULL);
			g_print ("horizontal binning  = %d (min:%d - max:%d)\n", value, minimum, maximum);
		}

		node = arv_gc_get_node (genicam, "BinningVertical");
		if (ARV_IS_GC_NODE (node)) {
			value = arv_gc_integer_get_value (ARV_GC_INTEGER (node), NULL);
			maximum = arv_gc_integer_get_max (ARV_GC_INTEGER (node), NULL);
			minimum = arv_gc_integer_get_min (ARV_GC_INTEGER (node), NULL);
			g_print ("vertical binning    = %d (min:%d - max:%d)\n", value, minimum, maximum);
		}

		node = arv_gc_get_node (genicam, "ExposureTimeAbs");
		if (ARV_IS_GC_NODE (node)) {
			v_double = arv_gc_float_get_value (ARV_GC_FLOAT (node), NULL);
			v_double_min = arv_gc_float_get_min (ARV_GC_FLOAT (node), NULL);
			v_double_max = arv_gc_float_get_max (ARV_GC_FLOAT (node), NULL);
			g_print ("exposure            = %g (min:%g - max:%g)\n", v_double, v_double_min, v_double_max);
		}

		node = arv_gc_get_node (genicam, "ExposureAuto");
		if (ARV_IS_GC_NODE (node)) {
			v_string = arv_gc_enumeration_get_string_value (ARV_GC_ENUMERATION (node), NULL);
			g_print ("exposure auto mode  = %s\n", v_string);
		}

		node = arv_gc_get_node (genicam, "GainRaw");
		if (ARV_IS_GC_NODE (node)) {
			value = arv_gc_integer_get_value (ARV_GC_INTEGER (node), NULL);
			maximum = arv_gc_integer_get_max (ARV_GC_INTEGER (node), NULL);
			minimum = arv_gc_integer_get_min (ARV_GC_INTEGER (node), NULL);
			g_print ("gain                = %d (min:%d - max:%d)\n", value, minimum, maximum);
		}

		node = arv_gc_get_node (genicam, "GainAuto");
		if (ARV_IS_GC_NODE (node)) {
			v_string = arv_gc_enumeration_get_string_value (ARV_GC_ENUMERATION (node), NULL);
			g_print ("gain auto mode      = %s\n", v_string);
		}

		node = arv_gc_get_node (genicam, "TriggerSelector");
		if (ARV_IS_GC_NODE (node)) {
			v_string = arv_gc_enumeration_get_string_value (ARV_GC_ENUMERATION (node), NULL);
			g_print ("trigger selector    = %s\n", v_string);
		}

		node = arv_gc_get_node (genicam, "ReverseX");
		if (ARV_IS_GC_NODE (node)) {
			v_boolean = arv_gc_boolean_get_value (ARV_GC_BOOLEAN (node), NULL);
			g_print ("reverse x           = %s\n", v_boolean ? "TRUE" : "FALSE");
		}

		stream = arv_device_create_stream (device, NULL, NULL, NULL);
		if (arv_option_auto_buffer)
			g_object_set (stream,
				      "socket-buffer", ARV_GV_STREAM_SOCKET_BUFFER_AUTO,
				      "socket-buffer-size", 0,
				      NULL);

		node = arv_gc_get_node (genicam, "PayloadSize");
		value = arv_gc_integer_get_value (ARV_GC_INTEGER (node), NULL);
		g_print ("payload size  = %d (0x%x)\n", value, value);

		for (i = 0; i < 30; i++)
			arv_stream_push_buffer (stream, arv_buffer_new (value, NULL));

		if (ARV_IS_GV_DEVICE (device)) {
			arv_device_read_register (device, ARV_GVBS_STREAM_CHANNEL_0_PORT_OFFSET, &value, NULL);
			g_print ("stream port = %d (%d)\n", value, arv_gv_stream_get_port (ARV_GV_STREAM (stream)));
		}

		arv_device_read_memory (device, 0x00014150, 8, memory_buffer, NULL);
		arv_device_read_memory (device, 0x000000e8, 16, memory_buffer, NULL);
		arv_device_read_memory (device,
					ARV_GVBS_USER_DEFINED_NAME_OFFSET,
					ARV_GVBS_USER_DEFINED_NAME_SIZE, memory_buffer, NULL);

		node = arv_gc_get_node (genicam, "AcquisitionStart");
		arv_gc_command_execute (ARV_GC_COMMAND (node), NULL);

		signal (SIGINT, set_cancel);

		start_time = g_get_real_time ();
		do {
			g_usleep (100000);

			do  {
				buffer = arv_stream_try_pop_buffer (stream);
				if (buffer != NULL) {
					printf("Got buffer #%d\n", buffer_count);
					bufferToImage(buffer,imgNum);
					arv_stream_push_buffer (stream, buffer);
					buffer_count++;
					imgNum++;
				}
			} while (buffer != NULL && buffer_count < 20);

			
			time = g_get_real_time ();
			if (time - start_time > 1000000) {
				printf ("Frame rate = %d Hz\n", buffer_count);
				printf("Waiting for a buffer... \n");
				//buffer_count = 0;
				start_time = time;
			}
		
		} while (!cancel && buffer_count < 20);

		if (ARV_IS_GV_DEVICE (device)) {
			arv_device_read_register (device, ARV_GVBS_STREAM_CHANNEL_0_PORT_OFFSET, &value, NULL);
			g_print ("stream port = %d (%d)\n", value, arv_gv_stream_get_port (ARV_GV_STREAM (stream)));
		}

		arv_stream_get_statistics (stream, &n_processed_buffers, &n_failures, &n_underruns);

		g_print ("Processed buffers = %llu\n", (unsigned long long) n_processed_buffers);
		g_print ("Failures          = %llu\n", (unsigned long long) n_failures);
		g_print ("Underruns         = %llu\n", (unsigned long long) n_underruns);

		node = arv_gc_get_node (genicam, "AcquisitionStop");
		arv_gc_command_execute (ARV_GC_COMMAND (node), NULL);

		g_object_unref (stream);
		g_object_unref (device);
	} else {
		g_print ("No device found\n");
	}

	if (error != NULL) {
		/* An error occurred, display the corresponding message */
		printf ("Error: %s\n", error->message);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

void bufferToImage(ArvBuffer* frameBuffer, int imgNum){

	if(frameBuffer != NULL){

		/*
		// Create file name string
		int imgCount = 1;
		char fileName[2];
		sprintf(fileName, "%d", imgCount);
		char fileType[] = ".jpg";
		strcat(fileName,fileType);

		char imgStrPrefix[] = "output_";
		char imgStrSuffix[] = ".jpg";
		char imgStrBody[] = "N";
		sprintf(imgStrBody, "%d", imgCount);
		strSize = sizeof(imgStrPrefix);
		strSize += sizeof(imgStrBody);
		strSize += sizeof(imgStrSuffix);
		strcat(fileName, imgStrPrefix);
		strcat(fileName, imgStrBody);
		strcat(fileName, imgStrSuffix);
		
		printf("File string %s\n", fileName);
		*/
		
		/* Write bufferData to image */
		guint8* bufferData;
		size_t dataSize;
		MagickBooleanType writeStatus;
		MagickBooleanType stackStatus;
		unsigned long imgWidth = (unsigned long)arv_buffer_get_image_width (frameBuffer);
		unsigned long imgHeight = (unsigned long)arv_buffer_get_image_height (frameBuffer);
		bufferData = (void*)arv_buffer_get_data(frameBuffer,&dataSize);

		//char fileName[] = "";
		size_t nameLen;
		char namePrefix[] = "/home/caleb/Downloads/";
		nameLen = strlen(namePrefix);
		printf("%s\n",namePrefix);
		char nameBody[] = "00";
		sprintf(nameBody, "%d", imgNum);
		nameLen += strlen(nameBody);
		printf("%s\n",nameBody);
		char nameSuffix[] = ".jpg";
		nameLen += strlen(nameSuffix);
		printf("%s\n",nameSuffix);
		char fileName[nameLen + 1];
		strcat(fileName, namePrefix);
		strcat(fileName, nameBody);
		strcat(fileName, nameSuffix);

		printf("Final file name is: %s Length is:%d\n", fileName,nameLen);

		printf("==== Buffer information ==== \n");
		printf("Buffer status: %d\n",
			arv_buffer_get_status(buffer));
		printf("Buffer payload type: %d\n",
			arv_buffer_get_payload_type(buffer));
		printf("Buffer image data format: 0x%x\n",
			arv_buffer_get_image_pixel_format(buffer));
		printf("Buffer data size: %lu bytes \n", dataSize);
		printf("Buffer WxH : %lux%lu\n",imgWidth,imgHeight);
		printf("____________________________ \n");
		
		MagickWand *wand = NewMagickWand();
		stackStatus = MagickConstituteImage(wand,imgWidth,imgHeight,"I",CharPixel,bufferData);
		stackStatus = MagickSetImageDepth(wand, 8);
		stackStatus = MagickSetImageColorspace(wand, GRAYColorspace);
		stackStatus = MagickSetImageFormat(wand, "jpg");
		stackStatus = MagickSetImageExtent(wand, imgWidth, imgHeight);
		writeStatus = MagickWriteImage(wand, fileName);
		if (writeStatus){
			printf("... and saved buffer %d to image file ./%s @ %lux%lu\n", imgNum,fileName,imgWidth,imgHeight);
		} else {
			printf("... but failed to write image %d\n", imgNum);
			printf("... MagickWand stack status = %d\n", stackStatus);
		}
		ClearMagickWand(wand);
	}
}