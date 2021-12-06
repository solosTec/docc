
#include <rt/i18n.h>

#include <iterator>

namespace docruntime
{
	std::string get_name(cyng::io::language_codes lc, i18n::word_id id)
	{
		switch (id) {
		case i18n::FIGURE:
			switch (lc) {
			case cyng::io::LC_ES: return "gráfica";
			case cyng::io::LC_SV: return "illustration";
			case cyng::io::LC_PT: return "ilustração";
			case cyng::io::LC_DE: return "Abbildung";
//			case cyng::io::LC_BG: return "Фигура";
//			case cyng::io::LC_RU: return "рисунок";	// рис.
			case cyng::io::LC_UK: return "??????????";
				//			case cyng::io::LC_CS: return "czech";
			case cyng::io::LC_FI: return "kuva";
//			case cyng::io::LC_EL: return "εικόνα";
				//			case cyng::io::LC_HU: return "magyar";
			case cyng::io::LC_IS: return "útskýring";
			case cyng::io::LC_IT: return "figura";
			case cyng::io::LC_NN: return "illustrasjon";
//			case cyng::io::LC_JA: return "フィギュア";
			case cyng::io::LC_KO: return "??";
			case cyng::io::LC_FA: return "?????";
			case cyng::io::LC_FR: return "figure";
			case cyng::io::LC_PL: return "rysunek";
			case cyng::io::LC_SK: return "ilustrácie";
			case cyng::io::LC_HE: return "????";
			case cyng::io::LC_NL: return "afbeelding";
			case cyng::io::LC_EU: return "ilustrazioa";
				//			case cyng::io::LC_BR: return "breton";
				//			case cyng::io::LC_CA: return "catalan";
			case cyng::io::LC_HR: return "ilustracija";
			case cyng::io::LC_ET: return "näide";
			case cyng::io::LC_GL: return "ilustración";
			case cyng::io::LC_GA: return "léaráid";
			case cyng::io::LC_LA: return "illustratio";
				//			case cyng::io::LC_SE: return "samin";
			case cyng::io::LC_RO: return "ilustrare";
			case cyng::io::LC_GD: return "dealbh";
			case cyng::io::LC_TR: return "resim";
			case cyng::io::LC_CY: return "darlunio";
			default:
				break;
			}
			return "figure";
		case i18n::TABLE:
			switch (lc) {
			case cyng::io::LC_ES: return "mesa";
			case cyng::io::LC_SV: return "bord";
			case cyng::io::LC_PT: return "mesa";
			case cyng::io::LC_DE: return "Tabelle";
			case cyng::io::LC_BG: return "????";
			case cyng::io::LC_RU: return "????";
			case cyng::io::LC_UK: return "table";
				//			case cyng::io::LC_CS: return "czech";
			case cyng::io::LC_FI: return "pöytä";
			case cyng::io::LC_FR: return "table";
			case cyng::io::LC_EL: return "???????";
			case cyng::io::LC_HU: return "táblázat";
			case cyng::io::LC_IS: return "Taflan";
			case cyng::io::LC_IT: return "tavolo";
			case cyng::io::LC_NN: return "bord";
			case cyng::io::LC_JA: return "????";
			case cyng::io::LC_KO: return "???";
			case cyng::io::LC_FA: return "????";
			case cyng::io::LC_PL: return "stó?";
			case cyng::io::LC_SK: return "stôl";
			case cyng::io::LC_HE: return "?????";
			case cyng::io::LC_NL: return "tafel";
			case cyng::io::LC_EU: return "taula";
				//			case cyng::io::LC_BR: return "breton";
				//			case cyng::io::LC_CA: return "catalan";
			case cyng::io::LC_HR: return "stol";
			case cyng::io::LC_ET: return "tabel";
			case cyng::io::LC_GL: return "mesa";
			case cyng::io::LC_GA: return "tábla";
			case cyng::io::LC_LA: return "mensam";
				//			case cyng::io::LC_SE: return "samin";
			case cyng::io::LC_RO: return "tabel";
			case cyng::io::LC_GD: return "Clàr";
			case cyng::io::LC_TR: return "tablo";
			case cyng::io::LC_CY: return "tabl";
			default:
				break;
			}
			return "table";
		case i18n::TOC:
			switch (lc) {
			case cyng::io::LC_ES: return "Tabla de contenido";
			case cyng::io::LC_SV: return "Innehållsförteckning";
			case cyng::io::LC_PT: return "Índice";	//	Portuguese
			case cyng::io::LC_DE: return "Inhaltsverzeichnis";
				//case cyng::io::LC_BG: return "Съдържание";
				//case cyng::io::LC_RU: return "Оглавление";
			case cyng::io::LC_UK: return "Table of Contents";
				//			case cyng::io::LC_CS: return "czech";
			case cyng::io::LC_FI: return "Sisällysluettelo";	//	Finnish
			case cyng::io::LC_FR: return "Tableau";	//	French
			//case cyng::io::LC_EL: return "Πίνακας περιεχομένων";	//	greek
				//			case cyng::io::LC_HU: return "magyar";
			case cyng::io::LC_IS: return "Efnisyfirlit";	//	Icelandic
			case cyng::io::LC_IT: return "Sommario";	//	Italian
			case cyng::io::LC_NN: return "Innholdsfortegnelse";	//	Norwegian
			//case cyng::io::LC_JA: return "目次";
			//case cyng::io::LC_KO: return "목차";
			case cyng::io::LC_FA: return "?????";
			case cyng::io::LC_PL: return "??????????";
			case cyng::io::LC_SK: return "??????????";
			case cyng::io::LC_HE: return "????";
			case cyng::io::LC_NL: return "??????????";
			case cyng::io::LC_EU: return "??????????";
				//			case cyng::io::LC_BR: return "breton";
				//			case cyng::io::LC_CA: return "catalan";
			case cyng::io::LC_HR: return "??????????";
			case cyng::io::LC_ET: return "??????????";
			case cyng::io::LC_GL: return "??????????";
			case cyng::io::LC_GA: return "??????????";
			case cyng::io::LC_LA: return "Table of Contents";	//	Latin
				//			case cyng::io::LC_SE: return "samin";
			case cyng::io::LC_RO: return "Cuprins";
			case cyng::io::LC_GD: return "Táboa de contidos";		//	Gaelic
			//case cyng::io::LC_TR: return "İçindekiler";		//	Turkish
			case cyng::io::LC_CY: return "Tabl Cynnwys";	//	Welsh
			default:
				break;
			}
			return "Table of Contents";
		default:
			break;
		}
		return "";
	}

}

