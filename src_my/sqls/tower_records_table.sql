DROP TABLE IF EXISTS tower_records;
CREATE TABLE IF NOT EXISTS tower_records (
level INT UNSIGNED UNIQUE,
records BLOB
)
CHARACTER SET = utf8
ENGINE = InnoDB;

DELIMITER $$

DROP PROCEDURE IF EXISTS get_tower_records$$
CREATE PROCEDURE get_tower_records (IN param1 INT UNSIGNED)
BEGIN
SELECT records
FROM tower_records
WHERE level = param1;
END$$

DROP PROCEDURE IF EXISTS update_tower_records$$
CREATE PROCEDURE update_tower_records (IN param1 INT UNSIGNED, IN param2 BLOB)
BEGIN
INSERT INTO tower_records(level, records)
VALUES(param1, param2)
ON DUPLICATE KEY UPDATE
records = param2;
END$$
