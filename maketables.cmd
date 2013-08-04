/* REXX: */

do wid = 0 to 1
    w = 80 + 52*wid
    do h = 10 to 130
	'mode 'w','h
	'mytFont -ws dir'
    end
end
