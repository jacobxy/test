DROP TABLE IF EXISTS `worldboss`;
CREATE TABLE `worldboss` (
      `id` tinyint(3) unsigned NOT NULL,
      `npcId` int(10) unsigned NOT NULL,
      `level` tinyint(3) unsigned NOT NULL,
      `location` smallint(5) unsigned NOT NULL,
      `count` tinyint(5) unsigned NOT NULL,
) ENGINE=InnoDB DEFAULT CHARSET=utf8;