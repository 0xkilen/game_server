DROP TABLE IF EXISTS images_announcements;
CREATE TABLE IF NOT EXISTS images_announcements (
id INT UNSIGNED UNIQUE,
data BLOB
)
CHARACTER SET = utf8
ENGINE = InnoDB;

DELIMITER $$

DROP PROCEDURE IF EXISTS add_images_announcement$$
CREATE PROCEDURE add_images_announcement (IN param1 BLOB)
BEGIN
INSERT INTO images_announcements(id, data)
VALUES(1, param1);
END$$

DROP PROCEDURE IF EXISTS update_images_announcement$$
CREATE PROCEDURE update_images_announcement (IN param1 BLOB)
BEGIN
UPDATE images_announcements
SET data = param1
WHERE id = 1;
END$$

DROP PROCEDURE IF EXISTS load_images_announcement$$
CREATE PROCEDURE load_images_announcement ()
BEGIN
SELECT data
FROM images_announcements;
END$$

DELIMITER ;
