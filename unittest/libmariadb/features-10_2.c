/*
*/

#include "my_test.h"

static int execute_direct(MYSQL *mysql)
{
  int rc= 0;
  long i= 0;
  MYSQL_STMT *stmt;
  MYSQL_BIND bind;
  unsigned int param_count= 1;
  MYSQL_RES *res= NULL;

  stmt= mysql_stmt_init(mysql);

  rc= mariadb_stmt_execute_direct(stmt, "DROP TABLE IF EXISTS t1", -1);
  check_stmt_rc(rc, stmt);

  rc= mariadb_stmt_execute_direct(stmt, "CREATE TABLE t1 (a int)", -1);
  check_stmt_rc(rc, stmt);

  memset(&bind, 0, sizeof(MYSQL_BIND));

  bind.buffer= &i;
  bind.buffer_type= MYSQL_TYPE_LONG;
  bind.buffer_length= sizeof(long);

  mysql_stmt_close(stmt);
  stmt= mysql_stmt_init(mysql);
  mysql_stmt_attr_set(stmt, STMT_ATTR_PREBIND_PARAMS, &param_count);

  rc= mysql_stmt_bind_param(stmt, &bind);
  check_stmt_rc(rc, stmt);
  rc= mariadb_stmt_execute_direct(stmt, "INSERT INTO t1 VALUES (?)", -1);
  check_stmt_rc(rc, stmt);

  for (i=1; i < 1000; i++)
  {
    rc= mysql_stmt_execute(stmt);
    check_stmt_rc(rc, stmt);
  }
  rc= mysql_stmt_close(stmt);
  check_mysql_rc(rc, mysql);

  rc= mysql_query(mysql, "SELECT * FROM t1");
  check_mysql_rc(rc, mysql);

  res= mysql_store_result(mysql);
  FAIL_IF(mysql_num_rows(res) != 1000, "Expected 1000 rows");

  mysql_free_result(res);

  return OK;
}

static int execute_direct_example(MYSQL *mysql)
{
  MYSQL_STMT *stmt= mysql_stmt_init(mysql);
  MYSQL_BIND bind[2];
  int intval= 1;
  int param_count= 2;
  const char *strval= "execute_direct_example";

  /* Direct execution without parameters */
  if (mariadb_stmt_execute_direct(stmt, "DROP TABLE IF EXISTS execute_direct", -1))
    goto error;
  if (mariadb_stmt_execute_direct(stmt, "CREATE TABLE execute_direct (a int, b varchar(20))", -1))
    goto error;

  memset(bind, 0, sizeof(MYSQL_BIND) * 2);
  bind[0].buffer_type= MYSQL_TYPE_SHORT;
  bind[0].buffer= &intval;
  bind[1].buffer_type= MYSQL_TYPE_STRING;
  bind[1].buffer= (char *)strval;
  bind[1].buffer_length= (ulong)strlen(strval);

  /* set number of parameters */
  if (mysql_stmt_attr_set(stmt, STMT_ATTR_PREBIND_PARAMS, &param_count))
    goto error;

  /* bind parameters */
  if (mysql_stmt_bind_param(stmt, bind))
    goto error;

  if (mariadb_stmt_execute_direct(stmt, "INSERT INTO execute_direct VALUES (?,?)", -1))
    goto error;

  mysql_stmt_close(stmt);
  return OK;
error:
  printf("Error: %s\n", mysql_stmt_error(stmt));
  mysql_stmt_close(stmt);
  return FAIL;
}

struct my_tests_st my_tests[] = {
  {"execute_direct", execute_direct, TEST_CONNECTION_DEFAULT, 0,  NULL,  NULL},
  {"execute_direct_example", execute_direct_example, TEST_CONNECTION_DEFAULT, 0,  NULL,  NULL},
  {NULL, NULL, 0, 0, NULL, NULL}
};


int main(int argc, char **argv)
{

  mysql_library_init(0,0,NULL);

  if (argc > 1)
    get_options(argc, argv);

  get_envvars();

  run_tests(my_tests);

  mysql_server_end();
  return(exit_status());
}
