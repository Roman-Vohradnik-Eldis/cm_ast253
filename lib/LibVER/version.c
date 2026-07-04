#include <version.h>
#include <sys/stat.h>
#include <fcntl.h>

char * strdup (const char * s);
void bzero (void * s, size_t n);

version_args __make_version_args (const size_t cnt, ...)
{
	va_list args;
	va_start (args, cnt);

	version_args new_args = {
		.argc  = va_arg (args, int *),
		.argv = va_arg (args, char ***),
		.runtime_version = "",
		.runtime_alocated = false,
		.progname = NULL
	};

	size_t args_no = cnt > 2 ? cnt - 2 : 0;

	char * dest = NULL;
	size_t dest_len = 0;

	for (size_t arg = 0;  arg < args_no; arg++)
	{
		const char * arg_str = va_arg (args, char *);

		if (!arg_str)
		{
			continue;
		}

		size_t arg_len = strlen (arg_str);
		dest_len += arg_len + 2;

		dest = realloc (dest, dest_len);
		bzero (dest + (dest_len - 2 - arg_len), arg_len + 1);

		strncat (dest, arg_str, arg_len);
		strncat (dest, " ", arg_len);
	}

	if (dest)
	{
		new_args.runtime_version = dest;
		new_args.runtime_alocated = true;
	}

	va_end (args);

	return new_args;
}

// progname = "Foobar", versions "A:1.2.3"  -> "Foobar:A:1.2.3"
char * __get_versions (const char *progname, const char * versions)
{
	if (!progname)
	{
		return strdup (versions);
	}

	char * separator_pos = strstr (versions, ":");

	if (!separator_pos)
	{
		return strdup (versions);
	}

	// +2 -> :
	char * new_versions = malloc (strlen (progname) + strlen (versions) + 2);
	bzero (new_versions, strlen (progname) + strlen (versions) + 2);

	strcat (new_versions, progname);
	strcat (new_versions, ":");

	strncat (new_versions, versions, separator_pos - versions);
	strcat (new_versions, separator_pos);

	return new_versions;
}


void __version_exit_clean (void)
{
	char path[256];
	snprintf (path, sizeof (path), "/tmp/%d.version", getpid ());
	remove (path);
}

void __version_generic (int error_exit, version_args args)
{
	int * argc = args.argc;
	char *** argv = args.argv;
	const char * runtime_version = args.runtime_version;

	int argno = 1;
	char *arg = NULL;
	enum VERSION_TYPE type = FILE_VERSION;

	char version[1024];
	memset (version, '\0', sizeof (version));

	while (argno < *argc)
	{
		arg = (*argv)[argno];

		if (!strcmp (arg, "--version"))
		{
			type = ELDIS_VERSION;
			(*argc)--;
			memmove (*argv + argno, *argv + argno + 1, (*argc - argno) * sizeof (char *));
		}
		else if (!strcmp (arg, "--git-version"))
		{
			type = GIT_VERSION;
			(*argc)--;
			memmove (*argv + argno, *argv + argno + 1, (*argc - argno) * sizeof (char *));
		}
		else if (!strcmp (arg, "--eldis-version"))
		{
			type = FULL_VERSION;
			(*argc)--;
			memmove (*argv + argno, *argv + argno + 1, (*argc - argno) * sizeof (char *));
		}
		else
		{
			argno++;
		}
	}

	char * versions = __get_versions (args.progname, VERSIONS);
	char * git_versions = __get_versions (args.progname, GIT_VERSIONS);

	switch (type)
	{
		case ELDIS_VERSION:
		{
			snprintf (version, sizeof (version), "%s%s\n", versions, runtime_version);
			break;
		}
		case GIT_VERSION:
		{
			snprintf (version, sizeof (version), "%s\n", git_versions);
			break;
		}
		case FULL_VERSION:
		case FILE_VERSION:
		{
			snprintf (version, sizeof (version), "%s%s %s\n", 
				versions, runtime_version, OS_PRETTY_NAME);
			break;
		}
	}

	if (args.runtime_alocated)
	{
		free (args.runtime_version);
	}

	free (git_versions);
	free (versions);

	if (type == FILE_VERSION)
	{
		char path[256];
		snprintf (path, sizeof (path), "/tmp/%d.version", getpid ());

		mode_t mask = umask (0);

		FILE * file = fopen (path, "w");
		if (!file)
		{
			if (error_exit)
			{
				err (1, "error: cannot open file $%s", path);
			}
			else
			{
				printf ("error: cannot open file $%s\n", path);
			}
		}
		else
		{
			fprintf (file, "%s", version);
			fclose (file);
			//atexit (__version_exit_clean);
		}

		umask (mask);
	}
	else
	{
		fprintf (stdout, "%s", version);
		exit (0);
	}


}

void __version (version_args args)
{
  __version_generic (1, args);
}

void __version_no_exit (version_args args)
{
  __version_generic (0, args);
}
