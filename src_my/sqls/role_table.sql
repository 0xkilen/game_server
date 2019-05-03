DROP TABLE IF EXISTS roles;
CREATE TABLE IF NOT EXISTS roles (
username VARCHAR(128) UNIQUE NOT NULL,
gid BIGINT UNSIGNED UNIQUE NOT NULL,
name VARCHAR(32) UNIQUE NOT NULL,
data MEDIUMBLOB NOT NULL
)
CHARACTER SET = utf8
ENGINE = InnoDB;

DELIMITER $$

DROP PROCEDURE IF EXISTS add_role$$
CREATE PROCEDURE add_role (IN param1 VARCHAR(128), IN param2 BIGINT UNSIGNED, IN param3 VARCHAR(32), IN param4 MEDIUMBLOB)
BEGIN
INSERT INTO roles(username, gid, name, data)
VALUES(param1, param2, param3, param4);
END$$

DROP PROCEDURE IF EXISTS find_role$$
CREATE PROCEDURE find_role (IN param1 VARCHAR(128))
BEGIN
SELECT gid, name, data
FROM roles
WHERE username = param1;
END$$

DROP PROCEDURE IF EXISTS find_role_by_gid$$
CREATE PROCEDURE find_role_by_gid (IN param1 BIGINT UNSIGNED)
BEGIN
SELECT gid, name, data, username
FROM roles
WHERE gid = param1;
END$$

DROP PROCEDURE IF EXISTS find_role_by_name$$
CREATE PROCEDURE find_role_by_name (IN param1 VARCHAR(32))
BEGIN
SELECT gid, username, data
FROM roles
WHERE name = param1;
END$$

DROP PROCEDURE IF EXISTS get_roles$$
CREATE PROCEDURE get_roles (IN param1 INT, IN param2 INT)
BEGIN
SELECT username, gid, name, data
FROM roles
LIMIT param1, param2;
END$$

DROP PROCEDURE IF EXISTS update_role$$
CREATE PROCEDURE update_role (IN param1 BIGINT UNSIGNED, IN param2 VARCHAR(32), IN param3 MEDIUMBLOB)
BEGIN
UPDATE roles
SET name = param2, data = param3
WHERE gid = param1;
END$$

DROP PROCEDURE IF EXISTS load_gid_and_rolename$$
CREATE PROCEDURE load_gid_and_rolename ()
BEGIN
SELECT gid, name, username
FROM roles;
END$$

DELIMITER ;
