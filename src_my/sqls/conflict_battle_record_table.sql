DROP TABLE IF EXISTS conflict_battle_records;
CREATE TABLE IF NOT EXISTS conflict_battle_records (
id INT UNSIGNED UNIQUE AUTO_INCREMENT,
record BLOB
)
CHARACTER SET = utf8
ENGINE = InnoDB;

DELIMITER $$

DROP PROCEDURE IF EXISTS get_conflict_battle_records$$
CREATE PROCEDURE get_conflict_battle_records (IN param1 INT UNSIGNED)
BEGIN
SELECT record
FROM conflict_battle_records
WHERE id = param1;
END$$

DROP PROCEDURE IF EXISTS add_conflict_battle_record$$
CREATE PROCEDURE add_conflict_battle_record(IN param1 BLOB)
BEGIN
INSERT INTO conflict_battle_records(record)
VALUES (param1);
END$$
