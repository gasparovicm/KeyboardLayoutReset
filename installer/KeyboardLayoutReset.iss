; Inno Setup script for KeyboardLayoutReset. Built by build.bat into dist\.
; Per-user install: no administrator rights needed. The settings page writes
; HKCU\Software\{#AppName}, the same values the tray menu changes.
;
; Silent install with settings:
;   KeyboardLayoutReset-Setup-x.y.z.exe /SILENT /LAYOUT=en-US /IDLE=60

#define AppName "KeyboardLayoutReset"
#define AppExe "KeyboardLayoutReset.exe"
; Name used up to 0.2.0. The AppId is unchanged, so setup upgrades an old
; install in place and removes its leftovers (see [InstallDelete]).
#define OldName "LayoutReset"
#define AppVersion GetStringFileInfo(AddBackslash(SourcePath) + "..\bin\" + AppExe, "ProductVersion")

[Setup]
AppId={{C3437FFF-BC67-45C7-B530-91A4419A888E}
AppName={#AppName}
AppVersion={#AppVersion}
AppPublisher=Martin Gasparovic
AppPublisherURL=https://github.com/gasparovicm/KeyboardLayoutReset
DefaultDirName={localappdata}\Programs\{#AppName}
; Don't reuse the old install's LayoutReset folder.
UsePreviousAppDir=no
DisableProgramGroupPage=yes
PrivilegesRequired=lowest
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
OutputDir=..\dist
OutputBaseFilename={#AppName}-Setup-{#AppVersion}
SetupIconFile=..\src\app.ico
UninstallDisplayIcon={app}\{#AppExe}
WizardStyle=modern
Compression=lzma2
SolidCompression=yes
; A running copy is closed in [Code] (CloseRunningApp) instead of asking the
; user to close it; setup starts it again at the end.
CloseApplications=no
; Pick the installer language from the Windows display language; ask only
; when there is no match.
ShowLanguageDialog=auto
LanguageDetectionMethod=uilanguage

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"
Name: "bulgarian"; MessagesFile: "compiler:Languages\Bulgarian.isl"
Name: "czech"; MessagesFile: "compiler:Languages\Czech.isl"
Name: "danish"; MessagesFile: "compiler:Languages\Danish.isl"
Name: "dutch"; MessagesFile: "compiler:Languages\Dutch.isl"
Name: "finnish"; MessagesFile: "compiler:Languages\Finnish.isl"
Name: "french"; MessagesFile: "compiler:Languages\French.isl"
Name: "german"; MessagesFile: "compiler:Languages\German.isl"
Name: "hungarian"; MessagesFile: "compiler:Languages\Hungarian.isl"
Name: "italian"; MessagesFile: "compiler:Languages\Italian.isl"
Name: "norwegian"; MessagesFile: "compiler:Languages\Norwegian.isl"
Name: "polish"; MessagesFile: "compiler:Languages\Polish.isl"
Name: "portuguese"; MessagesFile: "compiler:Languages\Portuguese.isl"
Name: "slovak"; MessagesFile: "compiler:Languages\Slovak.isl"
Name: "slovenian"; MessagesFile: "compiler:Languages\Slovenian.isl"
Name: "spanish"; MessagesFile: "compiler:Languages\Spanish.isl"
Name: "swedish"; MessagesFile: "compiler:Languages\Swedish.isl"
Name: "ukrainian"; MessagesFile: "compiler:Languages\Ukrainian.isl"

[CustomMessages]
; Text of the settings page and tasks. Languages without a translation here
; (and without an Inno Setup translation) use English.

english.SettingsTitle=Settings
english.SettingsDesc=Choose the default keyboard and how long to wait before switching back to it.
english.KeyboardLabel=Default keyboard to switch back to:
english.KeyboardHint="en" means the first English keyboard you have installed. You can also type any language name (fr-FR) or a layout id (00000409). Both settings can be changed later from the tray icon menu.
english.IdleLabel=Switch after this many seconds without keyboard or mouse input (0 = switch every 5 seconds, even while typing):
english.IdleError=Enter the idle time as a number of seconds between 0 and 86400.
english.KeyboardError=Choose a default keyboard.
english.AutostartTask=Start %1 when I sign in to Windows
english.StillRunning=%1 is still running and could not be closed. Exit it from its tray icon and try again.

bulgarian.SettingsTitle=Настройки
bulgarian.SettingsDesc=Изберете клавиатурата по подразбиране и колко да се чака, преди да се превключи обратно към нея.
bulgarian.KeyboardLabel=Клавиатура, към която да се превключва:
bulgarian.KeyboardHint="en" означава първата инсталирана английска клавиатура. Можете да въведете и име на език (fr-FR) или идентификатор на подредба (00000409). И двете настройки могат да се променят по-късно от менюто на иконата в системната област.
bulgarian.IdleLabel=Превключване след толкова секунди без въвеждане от клавиатура или мишка (0 = превключване на всеки 5 секунди, дори докато пишете):
bulgarian.IdleError=Въведете времето на бездействие като брой секунди между 0 и 86400.
bulgarian.KeyboardError=Изберете клавиатура по подразбиране.
bulgarian.AutostartTask=Стартиране на %1 при влизане в Windows
bulgarian.StillRunning=%1 все още работи и не можа да бъде затворено. Излезте от него чрез иконата в системната област и опитайте отново.

czech.SettingsTitle=Nastavení
czech.SettingsDesc=Vyberte výchozí klávesnici a jak dlouho čekat, než se na ni přepne zpět.
czech.KeyboardLabel=Výchozí klávesnice, na kterou se má přepínat:
czech.KeyboardHint="en" znamená první nainstalovanou anglickou klávesnici. Můžete zadat také název jazyka (fr-FR) nebo ID rozložení (00000409). Obě nastavení lze později změnit v nabídce ikony v oznamovací oblasti.
czech.IdleLabel=Přepnout po tomto počtu sekund bez vstupu z klávesnice nebo myši (0 = přepínat každých 5 sekund, i během psaní):
czech.IdleError=Zadejte dobu nečinnosti jako počet sekund od 0 do 86400.
czech.KeyboardError=Vyberte výchozí klávesnici.
czech.AutostartTask=Spustit %1 při přihlášení do Windows
czech.StillRunning=%1 stále běží a nepodařilo se jej ukončit. Ukončete jej pomocí ikony v oznamovací oblasti a zkuste to znovu.

danish.SettingsTitle=Indstillinger
danish.SettingsDesc=Vælg standardtastaturet, og hvor længe der skal ventes, før der skiftes tilbage til det.
danish.KeyboardLabel=Standardtastatur, der skal skiftes tilbage til:
danish.KeyboardHint="en" betyder det første engelske tastatur, du har installeret. Du kan også skrive et sprognavn (fr-FR) eller et layout-id (00000409). Begge indstillinger kan ændres senere i menuen for ikonet i meddelelsesområdet.
danish.IdleLabel=Skift efter så mange sekunder uden tastatur- eller musinput (0 = skift hvert 5. sekund, også mens du skriver):
danish.IdleError=Angiv inaktivitetstiden som et antal sekunder mellem 0 og 86400.
danish.KeyboardError=Vælg et standardtastatur.
danish.AutostartTask=Start %1, når jeg logger på Windows
danish.StillRunning=%1 kører stadig og kunne ikke lukkes. Afslut programmet via ikonet i meddelelsesområdet, og prøv igen.

dutch.SettingsTitle=Instellingen
dutch.SettingsDesc=Kies het standaardtoetsenbord en hoe lang er gewacht wordt voordat ernaar wordt teruggeschakeld.
dutch.KeyboardLabel=Standaardtoetsenbord om naar terug te schakelen:
dutch.KeyboardHint="en" betekent het eerste Engelse toetsenbord dat u hebt geïnstalleerd. U kunt ook een taalnaam (fr-FR) of een indelings-id (00000409) invoeren. Beide instellingen kunnen later worden gewijzigd via het menu van het pictogram in het systeemvak.
dutch.IdleLabel=Overschakelen na dit aantal seconden zonder toetsenbord- of muisinvoer (0 = elke 5 seconden overschakelen, ook tijdens het typen):
dutch.IdleError=Voer de inactiviteitstijd in als een aantal seconden tussen 0 en 86400.
dutch.KeyboardError=Kies een standaardtoetsenbord.
dutch.AutostartTask=%1 starten wanneer ik me aanmeld bij Windows
dutch.StillRunning=%1 is nog actief en kon niet worden afgesloten. Sluit het af via het pictogram in het systeemvak en probeer het opnieuw.

finnish.SettingsTitle=Asetukset
finnish.SettingsDesc=Valitse oletusnäppäimistö ja kuinka kauan odotetaan ennen siihen palaamista.
finnish.KeyboardLabel=Oletusnäppäimistö, johon palataan:
finnish.KeyboardHint="en" tarkoittaa ensimmäistä asennettua englanninkielistä näppäimistöä. Voit kirjoittaa myös kielen nimen (fr-FR) tai asettelun tunnuksen (00000409). Molempia asetuksia voi muuttaa myöhemmin ilmaisinalueen kuvakkeen valikosta.
finnish.IdleLabel=Vaihda, kun näppäimistöä tai hiirtä ei ole käytetty näin moneen sekuntiin (0 = vaihda 5 sekunnin välein, myös kirjoitettaessa):
finnish.IdleError=Anna käyttämättömyysaika sekunteina väliltä 0–86400.
finnish.KeyboardError=Valitse oletusnäppäimistö.
finnish.AutostartTask=Käynnistä %1, kun kirjaudun Windowsiin
finnish.StillRunning=%1 on yhä käynnissä, eikä sitä voitu sulkea. Sulje se ilmaisinalueen kuvakkeesta ja yritä uudelleen.

french.SettingsTitle=Paramètres
french.SettingsDesc=Choisissez le clavier par défaut et le délai avant d’y revenir.
french.KeyboardLabel=Clavier par défaut vers lequel revenir :
french.KeyboardHint=« en » désigne le premier clavier anglais installé. Vous pouvez aussi saisir un nom de langue (fr-FR) ou un identifiant de disposition (00000409). Les deux paramètres peuvent être modifiés plus tard depuis le menu de l’icône dans la zone de notification.
french.IdleLabel=Basculer après ce nombre de secondes sans saisie au clavier ou à la souris (0 = basculer toutes les 5 secondes, même pendant la saisie) :
french.IdleError=Saisissez le délai d’inactivité en secondes, entre 0 et 86400.
french.KeyboardError=Choisissez un clavier par défaut.
french.AutostartTask=Démarrer %1 à l’ouverture de session Windows
french.StillRunning=%1 est toujours en cours d’exécution et n’a pas pu être fermé. Quittez-le depuis son icône dans la zone de notification, puis réessayez.

german.SettingsTitle=Einstellungen
german.SettingsDesc=Wählen Sie die Standardtastatur und wie lange gewartet wird, bevor zu ihr zurückgewechselt wird.
german.KeyboardLabel=Standardtastatur, zu der zurückgewechselt wird:
german.KeyboardHint="en" steht für die erste installierte englische Tastatur. Sie können auch einen Sprachnamen (fr-FR) oder eine Layout-ID (00000409) eingeben. Beide Einstellungen lassen sich später über das Menü des Symbols im Infobereich ändern.
german.IdleLabel=Nach so vielen Sekunden ohne Tastatur- oder Mauseingabe wechseln (0 = alle 5 Sekunden wechseln, auch während der Eingabe):
german.IdleError=Geben Sie die Leerlaufzeit als Anzahl Sekunden zwischen 0 und 86400 ein.
german.KeyboardError=Wählen Sie eine Standardtastatur.
german.AutostartTask=%1 bei der Anmeldung an Windows starten
german.StillRunning=%1 wird noch ausgeführt und konnte nicht beendet werden. Beenden Sie es über das Symbol im Infobereich und versuchen Sie es erneut.

hungarian.SettingsTitle=Beállítások
hungarian.SettingsDesc=Válassza ki az alapértelmezett billentyűzetet, és hogy mennyi idő után váltson vissza rá.
hungarian.KeyboardLabel=Alapértelmezett billentyűzet, amelyre vissza kell váltani:
hungarian.KeyboardHint=Az "en" az első telepített angol billentyűzetet jelenti. Megadhat nyelvnevet (fr-FR) vagy kiosztásazonosítót (00000409) is. Mindkét beállítás később módosítható a tálcaikon menüjéből.
hungarian.IdleLabel=Váltás ennyi másodperc billentyűzet- vagy egérhasználat nélküli idő után (0 = váltás 5 másodpercenként, gépelés közben is):
hungarian.IdleError=Adja meg a tétlenségi időt másodpercben, 0 és 86400 között.
hungarian.KeyboardError=Válasszon alapértelmezett billentyűzetet.
hungarian.AutostartTask=A(z) %1 indítása a Windowsba való bejelentkezéskor
hungarian.StillRunning=A(z) %1 még fut, és nem sikerült bezárni. Lépjen ki belőle a tálcaikon segítségével, majd próbálja újra.

italian.SettingsTitle=Impostazioni
italian.SettingsDesc=Scegli la tastiera predefinita e quanto attendere prima di tornare a essa.
italian.KeyboardLabel=Tastiera predefinita a cui tornare:
italian.KeyboardHint="en" indica la prima tastiera inglese installata. Puoi anche digitare un nome di lingua (fr-FR) o un ID di layout (00000409). Entrambe le impostazioni possono essere modificate in seguito dal menu dell'icona nell'area di notifica.
italian.IdleLabel=Passa dopo questo numero di secondi senza input da tastiera o mouse (0 = passa ogni 5 secondi, anche durante la digitazione):
italian.IdleError=Inserisci il tempo di inattività in secondi, tra 0 e 86400.
italian.KeyboardError=Scegli una tastiera predefinita.
italian.AutostartTask=Avvia %1 all'accesso a Windows
italian.StillRunning=%1 è ancora in esecuzione e non è stato possibile chiuderlo. Chiudilo dall'icona nell'area di notifica e riprova.

norwegian.SettingsTitle=Innstillinger
norwegian.SettingsDesc=Velg standardtastaturet og hvor lenge det skal ventes før det byttes tilbake til det.
norwegian.KeyboardLabel=Standardtastatur å bytte tilbake til:
norwegian.KeyboardHint="en" betyr det første engelske tastaturet du har installert. Du kan også skrive et språknavn (fr-FR) eller en oppsett-ID (00000409). Begge innstillingene kan endres senere fra menyen til ikonet i systemstatusfeltet.
norwegian.IdleLabel=Bytt etter så mange sekunder uten tastatur- eller musinndata (0 = bytt hvert 5. sekund, også mens du skriver):
norwegian.IdleError=Angi inaktivitetstiden som et antall sekunder mellom 0 og 86400.
norwegian.KeyboardError=Velg et standardtastatur.
norwegian.AutostartTask=Start %1 når jeg logger på Windows
norwegian.StillRunning=%1 kjører fortsatt og kunne ikke lukkes. Avslutt det fra ikonet i systemstatusfeltet, og prøv på nytt.

polish.SettingsTitle=Ustawienia
polish.SettingsDesc=Wybierz domyślną klawiaturę i czas oczekiwania przed powrotem do niej.
polish.KeyboardLabel=Domyślna klawiatura, na którą przełączać:
polish.KeyboardHint=„en” oznacza pierwszą zainstalowaną klawiaturę angielską. Możesz też wpisać nazwę języka (fr-FR) lub identyfikator układu (00000409). Oba ustawienia można później zmienić w menu ikony w obszarze powiadomień.
polish.IdleLabel=Przełącz po tylu sekundach bez użycia klawiatury lub myszy (0 = przełączaj co 5 sekund, nawet podczas pisania):
polish.IdleError=Wprowadź czas bezczynności jako liczbę sekund od 0 do 86400.
polish.KeyboardError=Wybierz domyślną klawiaturę.
polish.AutostartTask=Uruchamiaj %1 po zalogowaniu do systemu Windows
polish.StillRunning=Aplikacja %1 nadal działa i nie można jej zamknąć. Zamknij ją za pomocą ikony w obszarze powiadomień i spróbuj ponownie.

portuguese.SettingsTitle=Definições
portuguese.SettingsDesc=Escolha o teclado predefinido e quanto tempo esperar antes de voltar a ele.
portuguese.KeyboardLabel=Teclado predefinido para o qual voltar:
portuguese.KeyboardHint="en" significa o primeiro teclado inglês instalado. Também pode escrever um nome de idioma (fr-FR) ou um ID de esquema (00000409). Ambas as definições podem ser alteradas mais tarde no menu do ícone na área de notificação.
portuguese.IdleLabel=Mudar após este número de segundos sem utilização do teclado ou do rato (0 = mudar a cada 5 segundos, mesmo durante a escrita):
portuguese.IdleError=Introduza o tempo de inatividade em segundos, entre 0 e 86400.
portuguese.KeyboardError=Escolha um teclado predefinido.
portuguese.AutostartTask=Iniciar o %1 ao iniciar sessão no Windows
portuguese.StillRunning=O %1 ainda está em execução e não foi possível fechá-lo. Feche-o a partir do ícone na área de notificação e tente novamente.

slovak.SettingsTitle=Nastavenia
slovak.SettingsDesc=Vyberte predvolenú klávesnicu a ako dlho čakať, kým sa na ňu prepne späť.
slovak.KeyboardLabel=Predvolená klávesnica, na ktorú sa má prepínať:
slovak.KeyboardHint="en" znamená prvú nainštalovanú anglickú klávesnicu. Môžete zadať aj názov jazyka (fr-FR) alebo ID rozloženia (00000409). Obe nastavenia môžete neskôr zmeniť v ponuke ikony v oblasti oznámení.
slovak.IdleLabel=Prepnúť po tomto počte sekúnd bez vstupu z klávesnice alebo myši (0 = prepínať každých 5 sekúnd, aj počas písania):
slovak.IdleError=Zadajte čas nečinnosti ako počet sekúnd od 0 do 86400.
slovak.KeyboardError=Vyberte predvolenú klávesnicu.
slovak.AutostartTask=Spustiť %1 pri prihlásení do Windows
slovak.StillRunning=%1 stále beží a nepodarilo sa ho zavrieť. Ukončite ho cez ikonu v oblasti oznámení a skúste to znova.

slovenian.SettingsTitle=Nastavitve
slovenian.SettingsDesc=Izberite privzeto tipkovnico in koliko časa naj se čaka pred preklopom nazaj nanjo.
slovenian.KeyboardLabel=Privzeta tipkovnica, na katero se preklopi:
slovenian.KeyboardHint="en" pomeni prvo nameščeno angleško tipkovnico. Vnesete lahko tudi ime jezika (fr-FR) ali ID razporeditve (00000409). Obe nastavitvi lahko pozneje spremenite v meniju ikone v območju za obvestila.
slovenian.IdleLabel=Preklopi po toliko sekundah brez vnosa s tipkovnico ali miško (0 = preklopi vsakih 5 sekund, tudi med tipkanjem):
slovenian.IdleError=Vnesite čas nedejavnosti kot število sekund med 0 in 86400.
slovenian.KeyboardError=Izberite privzeto tipkovnico.
slovenian.AutostartTask=Zaženi %1 ob prijavi v Windows
slovenian.StillRunning=%1 se še izvaja in ga ni bilo mogoče zapreti. Zaprite ga prek ikone v območju za obvestila in poskusite znova.

spanish.SettingsTitle=Configuración
spanish.SettingsDesc=Elija el teclado predeterminado y cuánto esperar antes de volver a él.
spanish.KeyboardLabel=Teclado predeterminado al que volver:
spanish.KeyboardHint="en" significa el primer teclado inglés instalado. También puede escribir un nombre de idioma (fr-FR) o un identificador de distribución (00000409). Ambas opciones se pueden cambiar más adelante desde el menú del icono en el área de notificación.
spanish.IdleLabel=Cambiar tras este número de segundos sin actividad del teclado o del ratón (0 = cambiar cada 5 segundos, incluso mientras escribe):
spanish.IdleError=Introduzca el tiempo de inactividad en segundos, entre 0 y 86400.
spanish.KeyboardError=Elija un teclado predeterminado.
spanish.AutostartTask=Iniciar %1 al iniciar sesión en Windows
spanish.StillRunning=%1 sigue en ejecución y no se pudo cerrar. Ciérrelo desde su icono en el área de notificación e inténtelo de nuevo.

swedish.SettingsTitle=Inställningar
swedish.SettingsDesc=Välj standardtangentbord och hur länge det ska väntas innan det växlas tillbaka till det.
swedish.KeyboardLabel=Standardtangentbord att växla tillbaka till:
swedish.KeyboardHint="en" betyder det första engelska tangentbordet du har installerat. Du kan även ange ett språknamn (fr-FR) eller ett layout-id (00000409). Båda inställningarna kan ändras senare från menyn för ikonen i meddelandefältet.
swedish.IdleLabel=Växla efter så här många sekunder utan tangentbords- eller musindata (0 = växla var 5:e sekund, även medan du skriver):
swedish.IdleError=Ange inaktivitetstiden som ett antal sekunder mellan 0 och 86400.
swedish.KeyboardError=Välj ett standardtangentbord.
swedish.AutostartTask=Starta %1 när jag loggar in i Windows
swedish.StillRunning=%1 körs fortfarande och kunde inte stängas. Avsluta det från ikonen i meddelandefältet och försök igen.

ukrainian.SettingsTitle=Параметри
ukrainian.SettingsDesc=Виберіть типову клавіатуру та скільки чекати, перш ніж повернутися до неї.
ukrainian.KeyboardLabel=Типова клавіатура, на яку перемикати:
ukrainian.KeyboardHint="en" означає першу встановлену англійську клавіатуру. Також можна ввести назву мови (fr-FR) або ідентифікатор розкладки (00000409). Обидва параметри можна змінити пізніше в меню значка в області сповіщень.
ukrainian.IdleLabel=Перемикати після стількох секунд без введення з клавіатури чи миші (0 = перемикати кожні 5 секунд, навіть під час введення):
ukrainian.IdleError=Введіть час бездіяльності в секундах, від 0 до 86400.
ukrainian.KeyboardError=Виберіть типову клавіатуру.
ukrainian.AutostartTask=Запускати %1 під час входу у Windows
ukrainian.StillRunning=%1 досі працює, і його не вдалося закрити. Вийдіть із нього через значок в області сповіщень і повторіть спробу.

[Tasks]
Name: autostart; Description: "{cm:AutostartTask,{#AppName}}"

[InstallDelete]
Type: filesandordirs; Name: "{localappdata}\Programs\{#OldName}"
Type: files; Name: "{userstartup}\{#OldName}.lnk"
Type: files; Name: "{autoprograms}\{#OldName}.lnk"

[Files]
Source: "..\bin\{#AppExe}"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\README.md"; DestDir: "{app}"; Flags: ignoreversion

[Icons]
Name: "{autoprograms}\{#AppName}"; Filename: "{app}\{#AppExe}"
Name: "{userstartup}\{#AppName}"; Filename: "{app}\{#AppExe}"; \
  Tasks: autostart

[Registry]
Root: HKCU; Subkey: "Software\{#AppName}"; Flags: uninsdeletekey
Root: HKCU; Subkey: "Software\{#AppName}"; ValueType: string; ValueName: "Layout"; ValueData: "{code:GetLayout}"
Root: HKCU; Subkey: "Software\{#AppName}"; ValueType: dword; ValueName: "IdleSeconds"; ValueData: "{code:GetIdle}"

[Run]
Filename: "{app}\{#AppExe}"; Description: "{cm:LaunchProgram,{#AppName}}"; \
  Flags: nowait postinstall

[Code]
var
  SettingsPage: TWizardPage;
  LayoutCombo: TNewComboBox;
  IdleEdit: TNewEdit;

{ Command-line value (/LAYOUT=, /IDLE=) wins, then the current setting (which
  the tray menu may have changed), then the previous install's value. }
function InitialValue(const Name, RegName, Default: String): String;
var
  Idle: Cardinal;
begin
  Result := ExpandConstant('{param:' + Name + '}');
  if Result <> '' then
    Exit;
  if (RegName = 'IdleSeconds') and
    RegQueryDWordValue(HKCU, 'Software\{#AppName}', RegName, Idle) then
    Result := IntToStr(Idle)
  else if not ((RegName = 'Layout') and
    RegQueryStringValue(HKCU, 'Software\{#AppName}', RegName, Result) and (Result <> '')) then
    Result := GetPreviousData(Name, Default);
end;

{ Word-wrapped label; returns its bottom so the next control goes below it,
  whatever the translated text's length. }
function AddLabel(const Caption: String; Top: Integer): Integer;
var
  L: TNewStaticText;
begin
  L := TNewStaticText.Create(SettingsPage);
  L.Parent := SettingsPage.Surface;
  L.Top := Top;
  L.Width := SettingsPage.SurfaceWidth;
  L.WordWrap := True;
  L.AutoSize := True;
  L.Caption := Caption;
  Result := L.Top + L.Height + ScaleY(4);
end;

procedure InitializeWizard;
var
  Top: Integer;
begin
  SettingsPage := CreateCustomPage(wpSelectTasks, CustomMessage('SettingsTitle'),
    CustomMessage('SettingsDesc'));

  Top := AddLabel(CustomMessage('KeyboardLabel'), 0);
  LayoutCombo := TNewComboBox.Create(SettingsPage);
  LayoutCombo.Parent := SettingsPage.Surface;
  LayoutCombo.Top := Top;
  LayoutCombo.Width := ScaleX(200);
  LayoutCombo.Style := csDropDown;
  LayoutCombo.Items.Add('en');
  LayoutCombo.Items.Add('en-US');
  LayoutCombo.Items.Add('en-GB');
  LayoutCombo.Items.Add('sk-SK');
  LayoutCombo.Items.Add('cs-CZ');
  LayoutCombo.Items.Add('de-DE');
  LayoutCombo.Text := InitialValue('Layout', 'Layout', 'en');
  Top := AddLabel(CustomMessage('KeyboardHint'), LayoutCombo.Top + LayoutCombo.Height + ScaleY(6));

  Top := AddLabel(CustomMessage('IdleLabel'), Top + ScaleY(12));
  IdleEdit := TNewEdit.Create(SettingsPage);
  IdleEdit.Parent := SettingsPage.Surface;
  IdleEdit.Top := Top;
  IdleEdit.Width := ScaleX(80);
  IdleEdit.Text := InitialValue('Idle', 'IdleSeconds', '60');
end;

function NextButtonClick(CurPageID: Integer): Boolean;
var
  Idle: Integer;
begin
  Result := True;
  if CurPageID = SettingsPage.ID then
  begin
    Idle := StrToIntDef(Trim(IdleEdit.Text), -1);
    if (Idle < 0) or (Idle > 86400) then
    begin
      MsgBox(CustomMessage('IdleError'), mbError, MB_OK);
      Result := False;
    end
    else if Trim(LayoutCombo.Text) = '' then
    begin
      MsgBox(CustomMessage('KeyboardError'), mbError, MB_OK);
      Result := False;
    end;
  end;
end;

procedure RegisterPreviousData(PreviousDataKey: Integer);
begin
  SetPreviousData(PreviousDataKey, 'Layout', Trim(LayoutCombo.Text));
  SetPreviousData(PreviousDataKey, 'Idle', Trim(IdleEdit.Text));
end;

function GetLayout(Param: String): String;
begin
  Result := Trim(LayoutCombo.Text);
end;

function GetIdle(Param: String): String;
begin
  Result := Trim(IdleEdit.Text);
end;

const
  WM_CLOSE = $0010;

{ Asks a running copy to exit, like tray > Exit. The app's hidden window has
  its name as class name; its mutex lives until the process has exited and
  released the exe. }
procedure RequestClose(const Name: String);
var
  Wnd: HWND;
begin
  Wnd := FindWindowByClassName(Name);
  if Wnd <> 0 then
    PostMessage(Wnd, WM_CLOSE, 0, 0);
end;

function IsRunning(const Name: String): Boolean;
begin
  Result := (FindWindowByClassName(Name) <> 0) or CheckForMutexes('Local\' + Name);
end;

{ Closes the app, also under its old name, and waits for it. Returns False if
  it is still running after a few seconds. }
function CloseRunningApp: Boolean;
var
  I: Integer;
begin
  RequestClose('{#AppName}');
  RequestClose('{#OldName}');
  for I := 1 to 50 do
  begin
    Result := not IsRunning('{#AppName}') and not IsRunning('{#OldName}');
    if Result then
      Exit;
    Sleep(100);
  end;
end;

function PrepareToInstall(var NeedsRestart: Boolean): String;
begin
  Result := '';
  if not CloseRunningApp then
    Result := FmtMessage(CustomMessage('StillRunning'), ['{#AppName}']);
end;

function InitializeUninstall: Boolean;
begin
  Result := True;
  if not CloseRunningApp then
  begin
    MsgBox(FmtMessage(CustomMessage('StillRunning'), ['{#AppName}']), mbError, MB_OK);
    Result := False;
  end;
end;

{ Autostart unticked on an upgrade: remove the shortcut an earlier install added. }
procedure CurStepChanged(CurStep: TSetupStep);
begin
  if (CurStep = ssPostInstall) and not WizardIsTaskSelected('autostart') then
    DeleteFile(ExpandConstant('{userstartup}\{#AppName}.lnk'));
end;
