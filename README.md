# VUT FIT GUX

Projekt do předmětu Grafická uživatelská rozhraní v X Window na VUT FIT 1. semestr, zima

Vytvořte jednoduchý grafický editor, který umožňuje:

- vykreslovat body, úsečky, obdélníky a elipsy, kde obdélníky a elipsy mohou být nevyplněné nebo vyplněné s obrysem; u elipsy se stiskem tlačítka označí střed elipsy a tažením myší se určí její x-ový a y-ový poloměr; Při tažení se objekt vykresluje (může být libovolný typ čáry, barva, šířka)!
- nastavit šířku čáry s alespoň 3 různými hodnotami včetně 0; šířka čáry určuje i velikost bodů, proto u bodů pro šířku 0 použijte funkci XDrawPoint() a u jiných šířek funkci XFillArc(); (Maximální šířka musi byt alespoň 8bodů - např. 0-3-8 !)
- nastavit barvu kreslené čáry (aspoň 4, ne jen black/white!); stačí barva popředí a pozadí; (použije se při šrafování!)
- nastavit barvu pro vyplňování (aspoň 4, ne jen black/white!); opět stačí barva popředí a pozadí; (použilo by se pro vyplňování vzorkem)
- zvolit mezi plnou nebo čárkovanou čarou typu LineDoubleDash (ne OnOffDash!) - nemusí se vztahovat na čáru tloušťky 0 (někde čárkovaní u tloušťky 0 nefungovalo);
- smazat nakreslený obrázek tlačítkem nebo z aplikačního menu;
- ukončit aplikaci tlačítkem nebo z aplikačního menu;
- při opouštění aplikace musí být zobrazen dialog, který se ptá, zda skutečně skončit (aplikaci lze ukončit i zvolením Close v menu nabízeném WM!).
