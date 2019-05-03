DROP TABLE IF EXISTS gladiators;
CREATE TABLE IF NOT EXISTS gladiators (
gid BIGINT UNSIGNED NOT NULL UNIQUE,
data MEDIUMBLOB NOT NULL
)
CHARACTER SET = utf8
ENGINE = InnoDB;

DELIMITER $$

DROP PROCEDURE IF EXISTS add_gladiator$$
CREATE PROCEDURE add_gladiator (IN param1 BIGINT UNSIGNED, IN param2 MEDIUMBLOB)
BEGIN
INSERT INTO gladiators(gid, data)
VALUES(param1, param2);
END$$

DROP PROCEDURE IF EXISTS find_gladiator$$
CREATE PROCEDURE find_gladiator (IN param1 BIGINT UNSIGNED)
BEGIN
SELECT data
FROM gladiators
WHERE gid = param1;
END$$

DROP PROCEDURE IF EXISTS get_gladiators$$
CREATE PROCEDURE get_gladiators (IN param1 INT, IN param2 INT)
BEGIN
SELECT gid, data
FROM gladiators
LIMIT param1, param2;
END$$

DROP PROCEDURE IF EXISTS update_gladiator$$
CREATE PROCEDURE update_gladiator (IN param1 BIGINT UNSIGNED, IN param2 MEDIUMBLOB)
BEGIN
UPDATE gladiators
SET data = param2
WHERE gid = param1;
END$$

DELIMITER ;
