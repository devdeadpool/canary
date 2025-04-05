function onUpdateDatabase()
	logger.info("Updating database to version 15 (Rook sample and GOD player values)")
	-- Rook Sample
	db.query("UPDATE `players` SET `ninjutsu` = 2, `manaspent` = 5936, `skill_fuinjutsu` = 12, `skill_fuinjutsu_tries` = 155, `skill_bukijutsu` = 12, `skill_bukijutsu_tries` = 155, `skill_axe` = 12, `skill_axe_tries` = 155, `skill_genjutsu` = 12, `skill_genjutsu_tries` = 93 WHERE `id` = 1;")
	-- GOD
	db.query("UPDATE `players` SET `health` = 155, `healthmax` = 155, `experience` = 100, `looktype` = 75, `town_id` = 8 WHERE `id` = 6;")
end
