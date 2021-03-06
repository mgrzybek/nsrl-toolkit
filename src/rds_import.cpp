/**
 * Project: nsrl-toolkit
 * File name: rds_import.cpp
 * Description: describes the main source file of rds_import
 *
 * @author Mathieu Grzybek on 2012-10-24
 * @copyright 2012 Mathieu Grzybek. All rights reserved.
 * @version $Id: code-gpl-license.txt,v 1.2 2004/05/04 13:19:30 garry Exp $
 *
 * @see The GNU Public License (GPL) version 3 or higher
 *
 *
 * nsrl-toolkit is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include "rds_import.h"

int	main(int argc, char** argv) {
	if ( argc < 2 ) {
		std::cerr << "Arg is missing" << std::endl;
		usage();
		return EXIT_FAILURE;
	}

	/*
	 * Dealing with settings
	 */

	m_settings	settings;

	if ( argc > 2 ) {
		int c;
		while ((c = getopt (argc, argv, "h:u:p:d:s:f:")) != -1 ) {
			switch (c) {
				case 'h':
					settings.insert("hostname", optarg);
					break;
				case 'u':
					settings.insert("username", optarg);
					break;
				case 'p':
					settings.insert("password", optarg);
					break;
				case 'd':
					settings.insert("driver", optarg);
					break;
				case 's':
					settings.insert("database", optarg);
					break;
				case 'f':
					settings.insert("import_type", optarg);
					break;
				case '?':
					return EXIT_FAILURE;
			}
		}
	} else {
		QCoreApplication::setOrganizationName("nsrl_toolkit");
		QCoreApplication::setApplicationName("rds_import");

	}

	if ( check_settings(settings) == false )
		return EXIT_FAILURE;

	QSqlDatabase	db;
	t_result	result;
	qint64		total_time;

	QElapsedTimer	timer;

	// Create the db object
	if ( init_db(db, settings) == false )
		return EXIT_FAILURE;

	QSqlQuery	query(db);

	// Open stdin
	QFile	q_stdin;

	if ( not q_stdin.open(stdin, QIODevice::ReadOnly) ) {
		std::cerr << "Cannot open stdin" << std::endl;
		return EXIT_FAILURE;
	}

	/*
	 * Let's choose the tables to update
	 */

	timer.start();

	if ( settings.value("import_type").compare("file") == 0 )
		result = import_nsrl_file(q_stdin, db, query);

	if ( settings.value("import_type").compare("mfg") == 0 )
		result = import_nsrl_mfg(q_stdin, db, query);

	if ( settings.value("import_type").compare("os") == 0 )
		result = import_nsrl_os(q_stdin, db, query);

	if ( settings.value("import_type").compare("prod") == 0 )
		result = import_nsrl_prod(q_stdin, db, query);

	total_time = timer.elapsed();

	if ( result.processed_lines > 0 ) {
		std::cout << "Elapsed time: " << total_time << " milliseconds (" << (float)total_time / 1000.0 << " seconds)" << std::endl;
		std::cout << "Processed lines: " << result.processed_lines << std::endl;
		std::cout << "Speed: " << (float)result.processed_lines / ((float)total_time / 1000.0 ) << " rows / second" << std::endl;
	} else {
		std::cout << "No line returned" << std::endl;
	}

	/*
	 * Ending
	 */
	q_stdin.close();

	// Close and destroy the db object
	db.close();
	QSqlDatabase::removeDatabase("MYSQL");

	if ( result.success == true )
		return EXIT_SUCCESS;

	return EXIT_FAILURE;
}

bool	check_settings(const m_settings& settings) {
	if ( settings.contains("hostname") == false ) {
		std::cerr << "hostname is missing" << std::endl;
		return false;
	}

	if ( settings.contains("database") == false ) {
		std::cerr << "database is missing" << std::endl;
		return false;
	}

	if ( settings.contains("driver") == false ) {
		std::cerr << "driver is missing" << std::endl;
		return false;
	}

	if ( settings.contains("username") == false ) {
		std::cerr << "username is missing" << std::endl;
		return false;
	}

	if ( settings.contains("import_type") == false or settings.value("import_type").isEmpty() == true ) {
		std::cerr << "import_type is missing" << std::endl;
		return false;
	}

	return true;
}

void	usage() {
	std::cout << "rds_import" << std::endl;
	std::cout << "	Imports NSRL CSV files to a remote database, according to the configuration file (according to QSettings' behaviour ~/.config/nsrl_toolkit/rds_import or registry) or the given arguments" << std::endl;
	std::cout << "Usage:" << std::endl;
	std::cout << "	rds_import <import_type> | (-h <hostname> -s <database> -d <driver> -u <username> -p <password> -f <import_type>)" << std::endl;
	std::cout << "Synopsys:" << std::endl;
	std::cout << "	zcat nsrlfiletxt.zip | tail -n +2 | rds_import file" << std::endl;
	std::cout << "	tail -n +2 nsrlmfg.txt | rds_import mfg" << std::endl;
	std::cout << "	tail -n +2 nsrlos.txt | rds_import os" << std::endl;
	std::cout << "	tail -n +2 nsrlprod.txt | rds_import prod" << std::endl;
	std::cout << "Settings / Arguments:" << std::endl;
	std::cout << "	hostname=	the target host" << std::endl;
	std::cout << "	database=	the database to use" << std::endl;
	std::cout << "	driver= 	the database's type, please refer to http://qt-project.org/doc/qt-5.0/qtsql/sql-driver.html" << std::endl;
	std::cout << "	username=	the login" << std::endl;
	std::cout << "	password=	the target host" << std::endl;
	std::cout << "Arguments:" << std::endl;
	std::cout << "	import_type:	the type of NSRL input (file, mfg, os, prod)" << std::endl;
}

