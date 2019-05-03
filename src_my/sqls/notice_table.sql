DROP TABLE IF EXISTS notices;
CREATE TABLE IF NOT EXISTS notices (
gid BIGINT UNSIGNED NOT NULL UNIQUE,
rolegid BIGINT UNSIGNED NOT NULL,
already_read INT UNSIGNED NOT NULL,
notice BLOB NOT NULL,
INDEX (rolegid)
)
CHARACTER SET = utf8
ENGINE = InnoDB;

DELIMITER $$

DROP PROCEDURE IF EXISTS add_notice$$
CREATE PROCEDURE add_notice (IN param1 BIGINT UNSIGNED, IN param2 BIGINT UNSIGNED, IN param3 INT UNSIGNED, IN param4 BLOB)
BEGIN
INSERT INTO notices(gid, rolegid, already_read, notice)
VALUES(param1, param2, param3, param4);
END$$

DROP PROCEDURE IF EXISTS find_notice$$
CREATE PROCEDURE find_notice (IN param1 BIGINT UNSIGNED)
BEGIN
SELECT gid, already_read, notice
FROM notices
WHERE rolegid = param1;
END$$

DROP PROCEDURE IF EXISTS get_notice$$
CREATE PROCEDURE get_notice (IN param1 BIGINT UNSIGNED, IN param2 BIGINT UNSIGNED)
BEGIN
SELECT gid, already_read, notice
FROM notices
WHERE gid = param1 AND rolegid = param2;
END$$

DROP PROCEDURE IF EXISTS get_notices$$
CREATE PROCEDURE get_notices (IN param1 INT, IN param2 INT)
BEGIN
SELECT gid, rolegid, already_read, notice
FROM notices
LIMIT param1, param2;
END$$

DROP PROCEDURE IF EXISTS delete_notice$$
CREATE PROCEDURE delete_notice (IN param1 BIGINT UNSIGNED, IN param2 BIGINT UNSIGNED)
BEGIN
DELETE FROM notices
WHERE gid = param1 AND rolegid = param2;
END$$

DROP PROCEDURE IF EXISTS set_notice_already_read$$
CREATE PROCEDURE set_notice_already_read (IN param1 BIGINT UNSIGNED, IN param2 BIGINT UNSIGNED, IN param3 INT UNSIGNED)
BEGIN
UPDATE notices
SET already_read = param3
WHERE gid = param1 AND rolegid = param2;
END$$

DELIMITER ;
