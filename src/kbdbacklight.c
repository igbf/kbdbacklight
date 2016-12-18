#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>

#define BLBASEPATH	"/sys/class/leds/smc::kbd_backlight"
#define BLMAXPATH	BLBASEPATH "/max_brightness"
#define BLVALPATH	BLBASEPATH "/brightness"

/* Define possible commands */
typedef enum
{
	get,
	get_ratio,
	set,
	set_change,
	set_percent,
	set_percent_change
} cmd;

/* Print a short help message */
void
usage(void)
{

}

/* Convert a percentage of a value to an absolute value */
double
toabs(int a, int b)
{
	return (a * b) / 100;
}

/* Parse command line arguments */
cmd
parse_cmd(char** args)
{
	cmd action;

	if (strcmp(args[1], "get") == 0)
	{
		action = get;

		if (args[2])
		{
			if (strcmp(args[2], "ratio") == 0)
				action = get_ratio;
			else
			{
				usage();
				exit(EXIT_FAILURE);
			}
		}
	}
	else if (strcmp(args[1], "set") == 0)
	{
		if (!args[2])
		{
			usage();
			exit(EXIT_FAILURE);
		}

		action = set;

		if (args[2][0] == '+' || args[2][0] == '-')
			action = set_change;

		if (args[2][strlen(args[2]) - 1] == '%')
		{
			args[2][strlen(args[2]) - 1] = '\0';

			switch(action)
			{
				case set_change:
					action = set_percent_change;
					break;
				default:
					action = set_percent;
			}
		}
	}

	return action;
}

/* Read value from filesystem */
int
get_brightness(char* path)
{
	int val;
	FILE* file = fopen(path, "r");
	if (!file)
		err(EXIT_FAILURE, "Could not open path: %s", path);

	fscanf(file, "%i", &val);
	fclose(file);
	return val;
}

/* Write value to filesystem */
void
set_brightness(char* path, int val)
{
	FILE* file = fopen(path, "w");

	if (!file)
		err(EXIT_FAILURE, "Could not open path: %s", path);

	if (fprintf(file, "%i", val) < 0)
		err(EXIT_FAILURE, "Could not write to path");

	fclose(file);
}

int main(int argc, char** args)
{
	/* The user should have passed at least a single argument */
	if (argc < 2)
	{
		usage();
		exit(EXIT_FAILURE);
	}

	/* Parse the arguments */
	cmd action = parse_cmd(args);

	int val, maxval;

	switch(action)
	{
		case get:
			printf("%i\n", get_brightness(BLVALPATH));
			break;
		case get_ratio:
			printf("%i/%i\n", get_brightness(BLVALPATH), get_brightness(BLMAXPATH));
			break;
		case set:
			maxval = get_brightness(BLMAXPATH);
			val = (int)strtol(args[2], NULL, 10);

			if (val > maxval)
				val = maxval;
			if (val < 0)
				val = 0;

			set_brightness(BLVALPATH, val);
			break;
		case set_percent:
			maxval = get_brightness(BLMAXPATH);
			val = toabs((int)strtol(args[2], NULL, 10), maxval);

			if (val > maxval)
				val = maxval;
			if (val < 0)
				val = 0;

			set_brightness(BLVALPATH, val);
			break;
		case set_change:
			maxval = get_brightness(BLMAXPATH);
			val = (int)strtol(args[2], NULL, 10) + get_brightness(BLVALPATH);

			if (val > maxval)
				val = maxval;
			if (val < 0)
				val = 0;

			set_brightness(BLVALPATH, val);
			break;
		case set_percent_change:
			maxval = get_brightness(BLMAXPATH);
			val = toabs((int)strtol(args[2], NULL, 10), maxval) + get_brightness(BLVALPATH);

			if (val > maxval)
				val = maxval;
			if (val < 0)
				val = 0;

			set_brightness(BLVALPATH, val);
			break;
		default:
			usage();
			exit(EXIT_FAILURE);
	}

	return EXIT_SUCCESS;
}
