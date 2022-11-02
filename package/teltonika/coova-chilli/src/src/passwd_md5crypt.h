#ifndef PASSWD_MD5CRYPT_H__
#define PASSWD_MD5CRYPT_H__

#define CH_ZERO		'\0'

int make_md5_salt(char **salt_p);
char *hash_md5_with_salt(char *password, char *salt);
char *hash_md5(char *password);
char *extract_salt(char *hash);

#endif /* passwd_md5crypt.h */
