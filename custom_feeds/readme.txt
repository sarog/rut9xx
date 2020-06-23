Kaip pasileisti:
einam i wrt dira 
Istrinam sena luci
	./scripts/feeds uninstall luci

atsidarom feeds.conf faila (jeigu tokio nera tai tada feeds.conf.default)
pridedam eilute:
	src-link teltonikaLuci /home/zmel/Documents/Projects/rut5xx_wrt/custom_feeds/teltonika_luci/contrib/package
beabejo reikia pamodinti patha...

dabar turime susikure customini feeda, belieka ji instaliuoti...
	./scripts/feeds install teltonikaLuci

pakeitimai daromi i custom_feeds/teltonika_luci dira. Veliau norint kad jie atsirastu builde, reikia paupdatinti feeda
	./scripts/feeds update teltonikaLuci 
gali tekti leisti su root teisem.

