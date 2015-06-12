/*
   Be a text-based xload
*/

#include "tl.h"

/* structs */
struct s_conf
{
	int	use_lines;
	time_t	sleep_time;
	int	loadavg_to_use;
	int	all_loads;
};

struct s_env
{
	double	**history;
	WINDOW	 *win;
	char	 *spin_chars;
	int	  current_spin_char;
	int	  history_count;
	int	  win_x;
	int	  win_y;
	int	  resize_now;
};


/* globals */

struct s_conf conf;
struct s_env env;

/* Display the right way to use this program. If we got here, it's not
 * good. Just exit. */
void
tl_usage (int argc, char **argv)
{
	fprintf(stderr, "tl\n");
	fprintf(stderr, "Version: %s\n", strlen(TL_VERSION_NAME) > 9 ?
			TL_VERSION_NAME : TL_VERSION_ID);
	fprintf(stderr, "Usage: %s [-s delay]\n", argv[0]);
	fprintf(stderr, "	-3		display three lines (default)\n");
	fprintf(stderr, "	-c		display using columns\n");
	fprintf(stderr, "	-h		this stuff\n");
	fprintf(stderr, "	-l		display one line\n");
	fprintf(stderr, "	-s delay	time between updates\n");
	exit(1);
}

/* Get the current screen dimensions */
void
get_geom ( )
{
	struct winsize w;

	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
	env.win_x = w.ws_col;
	env.win_y = w.ws_row;
}

int
get_scale ( )
{
	int scale, x, i;

	scale = 1;

	for (i = 0; i < 3; i++)
	{
		for (x = 0; x < env.win_x && x < env.history_count; x++)
		{
			if (		env.history != NULL &&
					env.history[x] != NULL &&
					scale <= env.history[x][i]
			   )
			{
				scale = env.history[x][i] + 1;
			}
		}
	}

	return(scale);
}


void
draw_scale_lines(int scale, int rows_per_tick)
{
	int x, y;

	for (x = 0; x < env.win_x; x++)
	{
		for (y = env.win_y - rows_per_tick; y > 0; y = y - rows_per_tick)
		{
			mvprintw(y, x, ".");
		}
	}
}


void
draw_line(int x, int scale, int rows_per_tick, int loadavg_to_use)
{
	int height, last_height;

	height = -1;
	last_height = -1;

	if (rows_per_tick >= 4)
	{
		if (env.history != NULL && env.history[x] != NULL)
		{
			height = env.history[x][loadavg_to_use] *
				rows_per_tick;
			if (		x+1 < env.win_x &&
					x+1 < env.history_count &&
					env.history[x+1] != NULL
			   ) {

				last_height =
					env.history[x+1][loadavg_to_use]
					* rows_per_tick;
			}
		}
	} else {
		if (env.history != NULL && env.history[x] != NULL)
		{
			height = (env.win_y - 1) * env.history[x][loadavg_to_use] / scale;
			if (x+1 < env.history_count && env.history[x+1] != NULL)
			{
				last_height = (env.win_y - 1) *
					env.history[x+1][loadavg_to_use]
					/ scale;
			}
		}
	}

	if (height == last_height)
	{
		mvprintw(env.win_y - height - 1, env.win_x -x -1, "-");

	} else if (height > last_height)
	{
		mvprintw(env.win_y - height - 1, env.win_x -x -1, "/");

	} else {
		mvprintw(env.win_y - height -1, env.win_x -x -1, "\\");
	}

}


void
draw_bar (int x, int scale, int rows_per_tick,
		int loadavg_to_use)
{
	int y, height;

	height = -1;

	if (env.history == NULL || env.history[x] == NULL)
	{
		return;
	}

	if (rows_per_tick >= 4)
	{
		height = env.history[x][loadavg_to_use] * rows_per_tick;
	} else {
		height = env.win_y * env.history[x][loadavg_to_use] / scale;
	}


	for (y = 0; y < height; y++)
	{
		mvprintw(env.win_y - y - 1, env.win_x - x - 1, "*");
	}
}

/* draw the screen */
void
draw_screen ( )
{
	int scale, rows_per_tick, x, i;
	char hostname[MAXHOSTNAMELEN];

	scale = get_scale();

	wclear(env.win);

	if (gethostname(hostname, MAXHOSTNAMELEN) == -1)
	{
		strncpy(hostname, "localhost", MAXHOSTNAMELEN);
	}

	rows_per_tick = (env.win_y - 1) / scale;

	mvprintw(0, 0, "%s:", hostname);
	for (i = 0; i < 3; i++)
	{
		if (conf.all_loads || i == conf.loadavg_to_use)
		{
			if (env.history != NULL && env.history[0] != NULL)
			{
				printw(" %0.2f", env.history[0][i]);
			}
		}
	}

	if (rows_per_tick < 4)
	{
		printw(" -- scale: %d", scale);
	}

	if (conf.use_lines)
	{
		if (rows_per_tick >= 4)
		{
			draw_scale_lines(scale, rows_per_tick);
		}
		for (i = 0; i < 3; i++)
		{
			if (conf.all_loads || i == conf.loadavg_to_use)
			{
				for (x = (env.win_x < env.history_count ? env.win_x : env.history_count) - 1; x >= 0; x--)
				{
					draw_line(x, scale, rows_per_tick, i);
				}
			}
		}

	} else {
		for (x = (env.win_x < env.history_count ? env.win_x : env.history_count) - 1; x >= 0; x--)
		{
			draw_bar(x, scale, rows_per_tick,
					conf.loadavg_to_use);
		}
		if (rows_per_tick >= 4)
		{
			draw_scale_lines(scale, rows_per_tick);
		}

	}

	mvwprintw(env.win, 0, env.win_x - 3, "%c",
			env.spin_chars[env.current_spin_char++]);
	if (env.current_spin_char >= strlen(env.spin_chars))
		env.current_spin_char = 0;

	wmove(env.win, 0, env.win_x - 2);

	refresh();
}

/* the window has been resized. Do something about it here. */
void
tl_resize (int sigraised)
{
	env.resize_now = 1;
	return;
}

/* Gets called when we get a signal. Cleanup the curses spooge and exit
 * gracefully-ish. */
void
tl_cleanup (int sigraised)
{
	if (endwin() == ERR)
	{
		fprintf(stderr, "Can't endwin: %s\n", strerror(errno));
		exit(1);
	}

	if (sigraised != -1)
		fprintf(stderr, "Caught signal %d, cleaning up\n", sigraised);

	if (sigraised == SIGHUP || sigraised == SIGINT ||
		sigraised == SIGQUIT || sigraised == SIGKILL ||
		sigraised == SIGTERM || sigraised == -1)
	{
		exit(0);
	} else {
		abort();
	}
}

/* set up the signals */
inline void
trap_signals ( )
{
	int ch;

	/* Trap signals to cleanup as much as possible */
	for (ch = 0; ch < 256; ch++)
	{
		switch (ch)
		{
			case SIGABRT:
				continue;

			case SIGWINCH:
				signal(SIGWINCH, &tl_resize);
				continue;

			default:
				signal(ch, &tl_cleanup);
		}
	}
}

/* miscellaneous initialization stuff. Go through the args, setup the
 * screen. */
void
tl_init (int argc, char **argv)
{
	int ch;

	bzero(&env, sizeof(env));
	env.resize_now = 1;
	env.spin_chars = SPIN_CHARS;
	env.current_spin_char = 0;

	bzero(&conf, sizeof(conf));
	conf.use_lines = 1;
	conf.all_loads = 1;

	while ((ch = getopt(argc, argv, "3ch?ls:")) != -1)
	{
		switch (ch)
		{
			case '3':
				conf.use_lines = 1;
				conf.all_loads = 1;
				break;

			case 'c':
				conf.use_lines = 0;
				break;

			case 'h':
				tl_usage(argc, argv);
				break;

			case '?':
				tl_usage(argc, argv);
				break;

			case 'l':
				conf.all_loads = 0;
				conf.use_lines = 1;
				break;

			case 's':
				errno = 0;
				conf.sleep_time = strtol(optarg, NULL, 0);
				break;

			default:
				tl_usage(argc, argv);
		}
	}

	if (conf.sleep_time < 300)
	{
		conf.loadavg_to_use = 0;
	} else if (conf.sleep_time < 900)
	{
		conf.loadavg_to_use = 1;
	} else 
	{
		conf.loadavg_to_use = 2;
	}

	if (conf.sleep_time == 0)
	{
		conf.sleep_time = 5;
	}
}


/* Update the array of list of load averages. Most recent is at [0]. */
void
tl_add_load ( )
{
	double		**dummy,
			  loadavg[3];
	int		  i;

	getloadavg(loadavg, 3);

	if (env.history == NULL)
	{
		/* Should only happen on the first pass */
		env.history = malloc(sizeof(*env.history));
		if (env.history == NULL)
		{
			fprintf(stderr, "Can't malloc %d bytes: %s\n", 
					(int)sizeof(*env.history),
					strerror(errno));
			tl_cleanup(-1);
			abort();
		}
	}

	if (env.win_x > env.history_count)
	{
		dummy = realloc(env.history, sizeof(env.history) * env.win_x);
		if (dummy == NULL)
		{
			fprintf(stderr, "Can't malloc %d bytes: %s\n", (int)
					sizeof(env.history) * env.history_count,
					strerror(errno));
			tl_cleanup(-1);
			abort();
		}

		if (env.history != dummy)
		{
			env.history = dummy;
		}
		env.history_count++;
	}

	for (i = env.history_count - 1; i >= 1; i--)
	{
		env.history[i] = env.history[i - 1];
	}
	/* XXX what about using a memmove? */

	env.history[0] = malloc(sizeof(loadavg));
	if (env.history[0] == NULL)
	{
		fprintf(stderr, "Can't malloc %d bytes: %s\n",
				(int) sizeof(loadavg), strerror(errno));
		tl_cleanup(-1);
		abort();
	}
	env.history[0][0] = loadavg[0];
	env.history[0][1] = loadavg[1];
	env.history[0][2] = loadavg[2];
}

void
verify_load ( )
{
	int x;
	double d;

	if (env.history == NULL)
	{
		return;
	}

	for (x = 0; x < env.history_count; x++)
	{
		if (env.history[x] == NULL)
		{
			continue;
		}
		d = env.history[x][0];
		d = env.history[x][1];
		d = env.history[x][2];
	}
}


int
main (int argc, char **argv)
{
	struct timeval now, refresh;
	int retval;

	tl_init(argc, argv);
	gettimeofday(&now, NULL);
	memcpy(&refresh, &now, sizeof(refresh));

	while (1)
	{

		if (env.resize_now)
		{
			env.resize_now = 0;

			if (env.win != NULL)
			{
				endwin();
			}
			if ((env.win = initscr()) == NULL)
			{
				abort();
			}
			get_geom();
			trap_signals();
			halfdelay(50);
		}

		gettimeofday(&now, NULL);
		if (		now.tv_sec > refresh.tv_sec ||
				(now.tv_sec == refresh.tv_sec &&
				 now.tv_usec > refresh.tv_usec)
		   ) {
			tl_add_load();
			refresh.tv_sec += conf.sleep_time;
		}
		verify_load();
		draw_screen();

		if ((retval = getch()) != ERR)
		{
			if ((char)retval == 'q')
			{
				tl_cleanup(-1);
			}
		}
	}
}
