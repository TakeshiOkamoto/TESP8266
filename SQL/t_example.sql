CREATE TABLE IF NOT EXISTS `t_example` (
  `no` int(11) NOT NULL AUTO_INCREMENT COMMENT 'NO(自動連番)',
  `type` varchar(10) NOT NULL COMMENT '種類',
  `value` int(11) NOT NULL COMMENT '数値',
  `string` varchar(30) DEFAULT NULL COMMENT '文字列',
  `reg_date` datetime NOT NULL COMMENT '登録日',
  PRIMARY KEY (`no`) 
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8;
