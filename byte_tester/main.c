
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int main(int argc, char *argv[]) {

  char tmp[256], test_id[256], tag_id[256];
  unsigned char *binary_file, bytes[256];
  int file_size, end, byte_count, i, tag_start, tag_end, wrong_bytes;
  FILE *f, *fb;
  
  if (argc != 2 || argv == NULL) {
    fprintf(stderr, "\n");
    fprintf(stderr, "Byte tester 1.0\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "USAGE: %s <TESTS FILE>\n", argv[0]);
    fprintf(stderr, "\n");
    fprintf(stderr, "TESTS FILE FORMAT:\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "<BINARY FILE NAME>\n");
    fprintf(stderr, "<TEST ID> <TAG ID> START <BYTE 1> <BYTE 2> ... <BYTE N> END\n");
    fprintf(stderr, "<TEST ID> <TAG ID> START <BYTE 1> <BYTE 2> ... <BYTE N> END\n");
    fprintf(stderr, "...\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "If <TAG ID> is \"01\", then we test if the bytes between \"01>\" and \"<01\" match\n");
    fprintf(stderr, "with bytes 1...n\n");
    fprintf(stderr, "\n");
    return 1;
  }

  f = fopen(argv[1], "rb");

  if (f == NULL) {
    fprintf(stderr, "Error opening file \"%s\".\n", argv[1]);
    return 1;
  }

  fscanf(f, "%255s", tmp);

  /* read the binary file */
  fb = fopen(tmp, "rb");

  if (fb == NULL) {
    fprintf(stderr, "Error opening file \"%s\".\n", tmp);
    fclose(f);
    return 1;
  }

  fseek(fb, 0, SEEK_END);
  file_size = (int)ftell(fb);
  fseek(fb, 0, SEEK_SET);

  binary_file = calloc(file_size, 1);
  if (binary_file == NULL) {
    fprintf(stderr, "Error allocating memory for file \"%s\".\n", tmp);
    fclose(f);
    fclose(fb);
    return 1;
  }

  fread(binary_file, 1, file_size, fb);
  fclose(fb);

  /* execute the tests */
  end = 0;
  while (end == 0) {
    if (fscanf(f, "%255s", test_id) == EOF)
      break;
    if (fscanf(f, "%255s", tag_id) == EOF)
      break;

    if (strlen(tag_id) != 2) {
      fprintf(stderr, "Test \"%s\" FAILED - TAG ID must be exactly two characters long. Error in \"%s\".\n", test_id, tag_id);
      break;
    }

    if (fscanf(f, "%255s", tmp) == EOF)
      break;

    if (strcmp(tmp, "START") != 0) {
      fprintf(stderr, "Test \"%s\" FAILED - START is missing.\n", test_id);
      break;
    }

    byte_count = 0;

    while (1) {
      if (byte_count == 256) {
	fprintf(stderr, "Test \"%s\" FAILED - Each test can contain max 256 bytes.\n", test_id);
	end = 1;
	break;
      }
      
      if (fscanf(f, "%255s", tmp) == EOF) {
	end = 1;
	break;
      }
      
      if (strcmp(tmp, "END") == 0)
	break;

      if (strlen(tmp) == 1)
	bytes[byte_count] = tmp[0];
      else if (strlen(tmp) == 2)
	bytes[byte_count] = (unsigned char)strtol(tmp, NULL, 16);
      else {
	fprintf(stderr, "Test \"%s\" FAILED - Unknown data \"%s\" in test \"%s\"! Must either be a character, two character hexadecimal value or END.\n", test_id, tmp, test_id);
	end = 1;
	break;
      }

      byte_count++;
    }

    /* execute the test */
    for (i = 0; i < file_size - 3; i++) {
      if (binary_file[i] == tag_id[0] && binary_file[i+1] == tag_id[1] && binary_file[i+2] == '>')
	break;
    }

    if (i == file_size - 3) {
      fprintf(stderr, "Test \"%s\" FAILED - Could not find tag \"%s>\".\n", test_id, tag_id);
      continue;
    }

    tag_start = i+3;

    for ( ; i < file_size - 3; i++) {
      if (binary_file[i] == '<' && binary_file[i+1] == tag_id[0] && binary_file[i+2] == tag_id[1])
	break;
    }

    if (i == file_size - 3) {
      fprintf(stderr, "Test \"%s\" FAILED - Could not find tag \"<%s\".\n", test_id, tag_id);
      continue;
    }

    tag_end = i;

    if (tag_end - tag_start != byte_count) {
      fprintf(stderr, "Test \"%s\" FAILED - There is %d bytes between the tags \"%s\", but the test \"%s\" defines only %d bytes.\n", test_id, tag_end - tag_start, tag_id, test_id, byte_count);
      continue;
    }

    /* compare bytes */
    wrong_bytes = 0;
    for (i = 0; i < byte_count; i++) {
      if (bytes[i] != binary_file[tag_start + i]) {
	if (wrong_bytes == 0)
	  fprintf(stderr, "Test \"%s\" FAILED - Bytes that don't match: %d", test_id, (i+1));
	else
	  fprintf(stderr, ", %d", (i+1));
	wrong_bytes++;
      }
    }

    if (wrong_bytes > 0)
      fprintf(stderr, "\n");
    else
      fprintf(stderr, "Test \"%s\" SUCCEEDED!\n", test_id);
  }
  
  fclose(f);
  
  free(binary_file);
  
  return 0;
}
