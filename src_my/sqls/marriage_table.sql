DROP TABLE IF EXISTS marriage;
CREATE TABLE IF NOT EXISTS marriage (
gid BIGINT UNSIGNED NOT NULL UNIQUE,
data MEDIUMBLOB
)
CHARACTER SET = utf8
ENGINE = InnoDB;

DELIMITER $$

DROP PROCEDURE IF EXISTS get_marriage$$
CREATE PROCEDURE get_marriage ()
BEGIN
SELECT data
FROM marriage
WHERE gid = 1;
END$$

DROP PROCEDURE IF EXISTS save_marriage$$
CREATE PROCEDURE save_marriage (IN param1 MEDIUMBLOB)
BEGIN
INSERT INTO marriage(gid, data)
VALUES(1, param1)
ON DUPLICATE KEY UPDATE
data = param1;
END$$

DELIMITER ;
