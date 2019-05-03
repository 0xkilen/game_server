DROP TABLE IF EXISTS guanpin;
CREATE TABLE IF NOT EXISTS guanpin (
gid BIGINT UNSIGNED NOT NULL UNIQUE,
data MEDIUMBLOB NOT NULL
)
CHARACTER SET = utf8
ENGINE = InnoDB;

DELIMITER $$

DROP PROCEDURE IF EXISTS get_guanpin$$
CREATE PROCEDURE get_guanpin ()
BEGIN
SELECT data
FROM guanpin
WHERE gid = 1;
END$$

DROP PROCEDURE IF EXISTS save_guanpin$$
CREATE PROCEDURE save_guanpin (IN param1 MEDIUMBLOB)
BEGIN
INSERT INTO guanpin(gid, data)
VALUES(1, param1)
ON DUPLICATE KEY UPDATE
data = param1;
END$$

DELIMITER ;
