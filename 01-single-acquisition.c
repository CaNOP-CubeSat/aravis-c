/* SPDX-License-Identifier:Unlicense */

/* Aravis header */

#include <arv.h>

/* Standard headers */

#include <stdlib.h>
#include <stdio.h>

/* Imagemagick headers */
#include <ImageMagick-7/MagickWand/MagickWand.h>

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
		printf ("Found camera '%s'", arv_camera_get_model_name (camera, NULL));

		/* Acquire a single buffer */
		buffer = arv_camera_acquisition (camera, 0, &error);
		size_t dataSize;
		guint8* bufferData = (guint8*)arv_buffer_get_data(buffer,&dataSize);
		const char * deviceId = arv_camera_get_device_id(camera, &error);

		//TODO: Fix buffer acquisition issue.

		if (ARV_IS_BUFFER (buffer)) {
			/* Display some informations about the retrieved buffer */
			unsigned long imgWidth = (unsigned long)arv_buffer_get_image_width (buffer);
			unsigned long imgHeight = (unsigned long)arv_buffer_get_image_height (buffer);
			printf(" ID: %s\n", deviceId);
			printf ("Acquired %lu×%lu buffer\n",
				imgWidth, imgHeight);
			printf("Buffer information: \n");
			printf("Buffer status: %d\n",
				arv_buffer_get_status(buffer));
			printf("Buffer payload type: %d\n",
				arv_buffer_get_payload_type(buffer));
			printf("Buffer image data format: 0x%x\n",
				arv_buffer_get_image_pixel_format(buffer));
			printf("Buffer data size: %lu bytes \n", dataSize);

			//DEBUG: printf("Raw buffer data: %.*x",dataSize, bufferData);

			/* Send buffer to image file */
			//MagickConstituteImage() -> ConstituteImage() -> ImportImagePixels()-> ImportCharPixel().
			
			MagickWand *wand = NewMagickWand();
			MagickConstituteImage(wand,imgWidth,imgHeight,"I",CharPixel,bufferData);
			MagickSetImageDepth(wand, 8);
			MagickSetImageColorspace(wand, GRAYColorspace);
			MagickSetImageFormat(wand, "jpg");
			MagickSetImageExtent(wand, imgWidth, imgHeight);
			MagickWriteImage(wand, "output.jpg");
			printf("Saved buffer to image file... \n");
			ClearMagickWand(wand);

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
