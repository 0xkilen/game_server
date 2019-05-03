DROP TABLE IF EXISTS system_announcements;
CREATE TABLE IF NOT EXISTS system_announcements (
id INT UNSIGNED UNIQUE,
data BLOB
)
CHARACTER SET = utf8
ENGINE = InnoDB;

DELIMITER $$

DROP PROCEDURE IF EXISTS add_announcement$$
CREATE PROCEDURE add_announcement (IN param1 INT UNSIGNED, IN param2 BLOB)
BEGIN
INSERT INTO system_announcements(id, data)
VALUES(param1, param2);
END$$

DROP PROCEDURE IF EXISTS delete_announcement$$
CREATE PROCEDURE delete_announcement (IN param1 INT UNSIGNED)
BEGIN
DELETE FROM system_announcements
WHERE id = param1;
END$$

DROP PROCEDURE IF EXISTS load_announcement$$
CREATE PROCEDURE load_announcement ()
BEGIN
SELECT id, data
FROM system_announcements;
END$$

DELIMITER ;
