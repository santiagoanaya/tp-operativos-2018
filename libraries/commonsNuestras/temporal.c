/*
 * Copyright (C) 2012 Sistemas Operativos - UTN FRBA. All rights reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "../commonsNuestras/temporal.h"

#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/timeb.h>
#include <string.h>

#include "../commonsNuestras/error.h"
#include "../commonsNuestras/string.h"

char *temporal_get_string_time() {
	struct tm *log_tm = malloc(sizeof(struct tm));
	char *str_time = string_duplicate("hh:mm:ss:mmmm");
	struct timeb tmili;
	time_t log_time;

	if ((log_time = time(NULL)) == -1) {
		error_show("Error getting date!");
		return 0;
	}

	localtime_r(&log_time, log_tm);

	if (ftime(&tmili)) {
		error_show("Error getting time!");
		return 0;
	}

	char *partial_time = string_duplicate("hh:mm:ss");
	strftime(partial_time, 127, "%H:%M:%S", log_tm);
	sprintf(str_time, "%s:%hu", partial_time, tmili.millitm);
	free(partial_time);
	free(log_tm);

	//Adjust memory allocation
	str_time = realloc(str_time, strlen(str_time) + 1);
	return str_time;
}
