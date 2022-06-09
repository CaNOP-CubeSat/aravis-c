/* SPDX-License-Identifier:Unlicense */

/* Aravis header */

#include <arv.h>

/* Standard headers */

#include <stdlib.h>
#include <stdio.h>

/*
 * Connect to the first available camera, then acquire a single buffer.
 */

int
main (int argc, char **argv)
{
	ArvCamera *camera;
	ArvBuffer *buffer;
	GError *error = NULL;

	/* Connect to the first available camera */
	camera = arv_camera_new (NULL, &error);

	if (ARV_IS_CAMERA (camera)) {
		printf ("Found camera '%s'\n", arv_camera_get_model_name (camera, NULL));

		/* Acquire a single buffer */
		buffer = arv_camera_acquisition (camera, 0, &error);

		//TODO: Fix buffer acquisition issue.

		if (ARV_IS_BUFFER (buffer)) {
			/* Display some informations about the retrieved buffer */
			printf ("Acquired %d×%d buffer\n",
				arv_buffer_get_image_width (buffer),
				arv_buffer_get_image_height (buffer));
			printf("Buffer information: \n");
			printf("Buffer status: %d\n",
				arv_buffer_get_status(buffer));
			printf("Buffer payload type: %d\n",
				arv_buffer_get_payload_type(buffer));
			printf("Buffer image data format: 0x%x\n",
				arv_buffer_get_image_pixel_format(buffer));
			size_t dataSize;
			guint8* bufferData = (guint8*)arv_buffer_get_data(buffer,&dataSize);
			printf("Raw buffer data: %.*x",
				dataSize, bufferData);
			/* Destroy the buffer */
			g_clear_object (&buffer);
		}

		/* Destroy the camera instance */
		g_clear_object (&camera);
	}

	if (error != NULL) {
		/* An error occurred, display the correspdonding message */
		printf ("Error: %s\n", error->message);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
