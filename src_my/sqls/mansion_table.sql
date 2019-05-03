DROP TABLE IF EXISTS mansions;
CREATE TABLE IF NOT EXISTS mansions (
gid BIGINT UNSIGNED UNIQUE,
data MEDIUMBLOB
)
CHARACTER SET = utf8
ENGINE = InnoDB;

DELIMITER $$

DROP PROCEDURE IF EXISTS add_mansion$$
CREATE PROCEDURE add_mansion (IN param1 BIGINT UNSIGNED, IN param2 MEDIUMBLOB)
BEGIN
INSERT INTO mansions(gid, data)
VALUES(param1, param2);
END$$

DROP PROCEDURE IF EXISTS find_mansion$$
CREATE PROCEDURE find_mansion (IN param1 BIGINT UNSIGNED)
BEGIN
SELECT data
FROM mansions
WHERE gid = param1;
END$$

DROP PROCEDURE IF EXISTS get_mansions$$
CREATE PROCEDURE get_mansions (IN param1 INT, IN param2 INT)
BEGIN
SELECT gid, data
FROM mansions
LIMIT param1, param2;
END$$

DROP PROCEDURE IF EXISTS update_mansion$$
CREATE PROCEDURE update_mansion (IN param1 BIGINT UNSIGNED, IN param2 MEDIUMBLOB)
BEGIN
UPDATE mansions
SET data = param2
WHERE gid = param1;
END$$
