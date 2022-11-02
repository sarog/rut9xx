#ifndef __MIGRATE_H
#define __MIGRATE_H

#include <sqlite3.h>

enum migration { LEGACY, RUTOS };

#define DB_COPY_TABLE                                                          \
	"INSERT INTO %s (time, sim, modem, interface, rx, tx, name)"                 \
	" SELECT time, sim, modem, interface, rx, tx, name FROM %s;"

#define DB_COPY_TABLE_NO_MODEM                                                 \
	"INSERT INTO %s (time, sim, modem, interface, rx, tx, name)"                 \
	"SELECT time, case when sim = 0 then 2 else sim end, modem, interface, rx, tx, name FROM %s;"

#define DB_CHECK_COLUMN	\
	"SELECT COUNT(*) FROM pragma_table_info('%s') WHERE name='%s';"

#define DB_RENAME "ALTER TABLE %s RENAME TO %s;"
#define DB_ADD_COLUMN "ALTER TABLE %s ADD COLUMN %s;"
#define DB_UPDATE_NAME_COL "UPDATE %s SET name = '%s'" \
	" WHERE (name IS NULL or name = '') AND modem = '%s' AND sim = '%s';"
#define DB_UPDATE_MODEM_COL "UPDATE %s SET modem = '%s'" \
	" WHERE (modem IS NULL or modem = '');"
#define DB_SANITIZE "DELETE FROM %s WHERE name = '' OR name IS NULL;"
#define DB_COUNT_IFACES "SELECT count(*) AS count FROM %s" \
" WHERE modem = '%s' GROUP BY interface"

void md_fix_legacy_db(void);

#endif //__MIGRATE_H