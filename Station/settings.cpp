/*

Settings management implementation for SmartGarden system. This module handles EEPROM-based settings, including settings Get/Set
and settings/hardware config import from an INI file.

Most of the code is written by Tony-osp (http://tony-osp.dreamwidth.org/)

Portions came from sprinklers_pi 2013 Richard Zimmerman (schedule handling is lifted from sprinklers_avr code)


*/

#include "settings.h"
#include "port.h"
#include <string.h>
#include <stdlib.h>
#include "LocalBoard.h"
#include <IniFile.h>
#include "LocalUI.h"
#include "eepromMap.h"



extern LocalBoardParallel	lBoardParallel;		// local hardware handler
extern LocalBoardSerial		lBoardSerial;		// local hardware handler
extern OSLocalUI localUI;


void LoadZone(uint8_t num, FullZone * pZone)
{
        if (num < 0 || num >= GetNumZones())
                return;
        for (uint8_t i = 0; i < sizeof(FullZone); i++)
                *((char*) pZone + i) = EEPROM.read(ZONE_OFFSET + i + ZONE_INDEX * num);
}

void SaveZone(uint8_t num, const FullZone * pZone)
{
        if (num < 0 || num >= GetNumZones())
                return;
        for (uint8_t i = 0; i < sizeof(FullZone); i++)
                EEPROM.write(ZONE_OFFSET + i + ZONE_INDEX * num, *((char*) pZone + i));
}

void LoadShortZone(uint8_t num, ShortZone * pZone)
{
        if (num < 0 || num >= GetNumZones())
                return;
        for (uint8_t i = 0; i < sizeof(ShortZone); i++)
                *((char*) pZone + i) = EEPROM.read(ZONE_OFFSET + i + ZONE_INDEX * num);
}


uint8_t GetDirectIOPin(uint8_t n)
{
	return EEPROM.read(ADDR_OT_DIRECT_IO + n);
}

void LoadZoneIOMap(uint8_t *ptr)
{
        for( int i = 0; i < LOCAL_NUM_DIRECT_CHANNELS; i++)
                *(ptr + i) = EEPROM.read(ADDR_OT_DIRECT_IO + i);
}

void SaveZoneIOMap(uint8_t *ptr)
{
        for( int i = 0; i < LOCAL_NUM_DIRECT_CHANNELS; i++)
                EEPROM.write(ADDR_OT_DIRECT_IO + i, *(ptr+i));
}

void LoadSrIOMap(SrIOMapStruct *ptr)
{
	ptr->SrClkPin = EEPROM.read(ADDR_OT_OPEN_SPRINKLER);
	ptr->SrNoePin = EEPROM.read(ADDR_OT_OPEN_SPRINKLER+1);
	ptr->SrDatPin = EEPROM.read(ADDR_OT_OPEN_SPRINKLER+2);
	ptr->SrLatPin = EEPROM.read(ADDR_OT_OPEN_SPRINKLER+3);
}

void SaveSrIOMap(SrIOMapStruct *ptr)
{
	EEPROM.write(ADDR_OT_OPEN_SPRINKLER, ptr->SrClkPin);
	EEPROM.write(ADDR_OT_OPEN_SPRINKLER+1, ptr->SrNoePin);
	EEPROM.write(ADDR_OT_OPEN_SPRINKLER+2, ptr->SrDatPin);
	EEPROM.write(ADDR_OT_OPEN_SPRINKLER+3, ptr->SrLatPin);
}


void SetNumIOChannels(uint8_t nchannels)
{
	EEPROM.write(ADDR_NUM_OT_DIRECT_IO, nchannels);
}

uint8_t GetNumIOChannels(void)
{
	return EEPROM.read(ADDR_NUM_OT_DIRECT_IO);
}


void LoadStation(uint8_t num, FullStation *pStation)
{
        if( num >= MAX_STATIONS )
                return;
        for (uint8_t i = 0; i < sizeof(FullStation); i++)
                *((char*) pStation + i) = EEPROM.read(STATION_OFFSET + i + STATION_INDEX * num);
}

void LoadShortStation(uint8_t num, ShortStation *pStation)
{
        if( num >= MAX_STATIONS )
                return;
        for (uint8_t i = 0; i < sizeof(ShortStation); i++)
                *((char*) pStation + i) = EEPROM.read(STATION_OFFSET + i + STATION_INDEX * num);
}

void SaveStation(uint8_t num, FullStation * pStation)
{
        if( num >= MAX_STATIONS )
                return;
        for (uint8_t i = 0; i < sizeof(FullStation); i++)
                EEPROM.write(STATION_OFFSET + i + STATION_INDEX * num, *((char*) pStation + i));
}

void SaveShortStation(uint8_t num, ShortStation * pStation)
{
        if( num >= MAX_STATIONS )
                return;
        for (uint8_t i = 0; i < sizeof(ShortStation); i++)
                EEPROM.write(STATION_OFFSET + i + STATION_INDEX * num, *((char*) pStation + i));
}


void LoadSensor(uint8_t num, FullSensor *pSensor)
{
        if( num >= MAX_SENSORS )
                return;
        for (uint8_t i = 0; i < sizeof(FullSensor); i++)
                *((char*) pSensor + i) = EEPROM.read(SENSOR_OFFSET + i + SENSOR_INDEX * num);
}

void LoadShortSensor(uint8_t num, ShortSensor *pSensor)
{
        if( num >= MAX_SENSORS )
                return;
        for (uint8_t i = 0; i < sizeof(ShortSensor); i++)
                *((char*) pSensor + i) = EEPROM.read(SENSOR_OFFSET + i + SENSOR_INDEX * num);
}


void SaveSensor(uint8_t num, FullSensor * pSensor)
{
        if( num >= MAX_SENSORS )
                return;
        for (uint8_t i = 0; i < sizeof(FullSensor); i++)
                EEPROM.write(SENSOR_OFFSET + i + SENSOR_INDEX * num, *((char*) pSensor + i));
}

void SaveShortSensor(uint8_t num, ShortSensor * pSensor)
{
        if( num >= MAX_SENSORS )
                return;
        for (uint8_t i = 0; i < sizeof(ShortSensor); i++)
                EEPROM.write(SENSOR_OFFSET + i + SENSOR_INDEX * num, *((char*) pSensor + i));
}


Schedule::Schedule() : m_type(0), day(0)
{
        name[0] = 0;
        for (uint8_t i=0; i<sizeof(time)/sizeof(time[0]); i++)
                time[i] = -1;
        for (uint8_t i=0; i<sizeof(zone_duration)/sizeof(zone_duration[0]); i++)
                zone_duration[i] = 0;
}

void LoadSchedule(uint8_t num, Schedule * pSched)
{
        if (num < 0 || num >= MAX_SCHEDULES)
                return;
        for (uint8_t i = 0; i < sizeof(Schedule); ++i)
        {
                *(((char*) pSched) + i) = EEPROM.read(SCHEDULE_OFFSET + i + SCHEDULE_INDEX * num);
        }
}

void SaveSchedule(uint8_t num, const Schedule * pSched)
{
        if (num < 0 || num >= MAX_SCHEDULES)
                return;
        for (uint8_t i = 0; i < sizeof(Schedule); i++)
                EEPROM.write(SCHEDULE_OFFSET + i + SCHEDULE_INDEX * num, *((char*) pSched + i));
}



uint8_t GetNumZones(void)
{
	return EEPROM.read(ADDR_NUM_ZONES);
}

uint8_t GetPumpStation(void)
{
	return EEPROM.read(ADDR_PUMP_STATION);
}

uint8_t GetPumpChannel(void)
{
	return EEPROM.read(ADDR_PUMP_CHANNEL);
}


void SetNumZones(uint8_t numZones)
{
	EEPROM.write(ADDR_NUM_ZONES, numZones);
}

void SetPumpStation(uint8_t pumpStation)
{
	EEPROM.write(ADDR_PUMP_STATION, pumpStation);
}

void SetPumpChannel(uint8_t pumpChannel)
{
	EEPROM.write(ADDR_PUMP_CHANNEL, pumpChannel);
}


bool IsPumpEnabled(void)
{
	return (EEPROM.read(ADDR_PUMP_STATION) == 255) ? true:false;
}

void SetNumOSChannels(uint8_t nchannels)
{
	EEPROM.write(ADDR_NUM_OT_OPEN_SPRINKLER, nchannels);
}

uint8_t GetNumOSChannels(void)
{
	return EEPROM.read(ADDR_NUM_OT_OPEN_SPRINKLER);
}

uint8_t GetXBeeFlags(void)
{
	return EEPROM.read(ADDR_NETWORK_XBEE_FLAGS);
}

bool IsXBeeEnabled(void)
{
	return (EEPROM.read(ADDR_NETWORK_XBEE_FLAGS) & NETWORK_FLAGS_ENABLED) ? true:false;
}

uint8_t GetXBeeChan(void)
{
	return EEPROM.read(ADDR_NETWORK_XBEE_CHAN);
}

uint8_t GetXBeePort(void)
{
	return EEPROM.read(ADDR_NETWORK_XBEE_PORT);
}
uint16_t GetXBeePortSpeed(void)
{
	uint16_t speed;

	speed = EEPROM.read(ADDR_NETWORK_XBEE_SPEED+1) << 8;
	speed += EEPROM.read(ADDR_NETWORK_XBEE_SPEED);

	return speed;
}

uint16_t GetXBeePANID(void)
{
	uint16_t panID;

	panID = EEPROM.read(ADDR_NETWORK_XBEE_PANID+1) << 8;
	panID += EEPROM.read(ADDR_NETWORK_XBEE_PANID);

	return panID;
}

uint16_t GetXBeeAddr(void)
{
	uint16_t addr;

	addr = EEPROM.read(ADDR_NETWORK_XBEE_ADDR16+1) << 8;
	addr += EEPROM.read(ADDR_NETWORK_XBEE_ADDR16);

	return addr;
}

void SetXBeeFlags(uint8_t flags)
{
	EEPROM.write(ADDR_NETWORK_XBEE_FLAGS, flags);
}


void SetXBeeChan(uint8_t chan)
{
	EEPROM.write(ADDR_NETWORK_XBEE_CHAN, chan);
}

void SetXBeePort(uint8_t port)
{
	EEPROM.write(ADDR_NETWORK_XBEE_PORT, port);
}

void SetXBeePortSpeed(uint16_t speed)
{
	uint8_t speedh = (speed & 0x0FF00) >> 8;
	uint8_t speedl = speed & 0x0FF;

	EEPROM.write(ADDR_NETWORK_XBEE_SPEED, speedl);
	EEPROM.write(ADDR_NETWORK_XBEE_SPEED+1, speedh);
}

void SetXBeePANID(uint16_t panID)
{
	uint8_t panIDh = (panID & 0x0FF00) >> 8;
	uint8_t panIDl = panID & 0x0FF;

	EEPROM.write(ADDR_NETWORK_XBEE_PANID, panIDl);
	EEPROM.write(ADDR_NETWORK_XBEE_PANID+1, panIDh);
}

void SetXBeeAddr(uint16_t addr)
{
	uint8_t addrh = (addr & 0x0FF00) >> 8;
	uint8_t addrl = addr & 0x0FF;

	EEPROM.write(ADDR_NETWORK_XBEE_ADDR16, addrl);
	EEPROM.write(ADDR_NETWORK_XBEE_ADDR16+1, addrh);
}

uint8_t GetMyStationID(void)
{
	return EEPROM.read(ADDR_MY_STATION_ID);
}

void SetMyStationID(uint8_t stationID)
{
	EEPROM.write(ADDR_MY_STATION_ID, stationID);
}



// Decode an IP address in dotted decimal format.
static IPAddress decodeIP(const char * value)
{
        uint8_t ip[4];
        const char * pEnd = value;
        int i = 0;
        while (i < 4)
        {
                ip[i++] = strtoul(pEnd, (char**) &pEnd, 10);
                if (!pEnd || (*pEnd++ != '.'))
                        break;
        }
        if (i == 4)
                return IPAddress(ip[0], ip[1], ip[2], ip[3]);
        else
                return INADDR_NONE;
}

//************************************
// Method:    SetSchedule
// FullName:  SetSchedule
// Access:    public
// Returns:   bool
// Qualifier:
// Parameter: const KVPairs & key_value_pairs
//************************************
bool SetSchedule(const KVPairs & key_value_pairs)
{
        freeMemory();
        Schedule sched;
        int sched_num = -1;
        sched.day = 0;
        sched.time[0] = -1;
        sched.time[1] = -1;
        sched.time[2] = -1;
        sched.time[3] = -1;
        bool time_enable[4] = {0};

        // Iterate through the kv pairs and update the appropriate structure values.
        for (int i = 0; i < key_value_pairs.num_pairs; i++)
        {
                const char * key = key_value_pairs.keys[i];
                const char * value = key_value_pairs.values[i];
                if (strcmp_P(key, PSTR("id")) == 0)
                {
                        sched_num = atoi(value);
                }
                else if (strcmp_P(key, PSTR("type")) == 0)
                        sched.SetInterval(strcmp_P(value, PSTR("on")) != 0);
                else if (strcmp_P(key, PSTR("enable")) == 0)
                        sched.SetEnabled(strcmp_P(value, PSTR("on")) == 0);
                else if (strcmp_P(key, PSTR("wadj")) == 0)
                        sched.SetWAdj(strcmp_P(value, PSTR("on")) == 0);
                else if (strcmp_P(key, PSTR("name")) == 0)
                        strncpy(sched.name, value, sizeof(sched.name));
                else if (strcmp_P(key, PSTR("interval")) == 0)
                {
                        if (sched.IsInterval())
                                sched.interval = atoi(value);
                }
                else if ((key[0] == 'd') && (key[2] == 0) && ((key[1] >= '1') && (key[1] <= '7')) && !(sched.IsInterval()))
                {
                        if (strcmp_P(value, PSTR("on")) == 0)
                                sched.day = sched.day | 0x01 << (key[1] - '1');
                        else
                                sched.day = sched.day & ~(0x01 << (key[1] - '1'));
                }
                else if ((key[0] == 't') && (key[2] == 0) && ((key[1] >= '1') && (key[1] <= '4')))
                {
                        const char * colon_loc = strstr(value, ":");
                        if (colon_loc > 0)
                        {
                                int hour = strtol(value, NULL, 10);
                                int minute = strtol(colon_loc + 1, NULL, 10);
                                bool bIsPM = strstr(value, "PM") || strstr(value, "pm");
                                if (bIsPM)
                                        hour += 12;
                                if ((hour >= 24) || (hour < 0) || (minute >= 60) || (minute < 0))
                                {
                                        trace(F("Invalid Date Input\n"));
                                        return false;
                                }
                                sched.time[key[1] - '1'] = hour * 60 + minute;
                        }
                }
                else if ((key[0] == 'e') && (key[2] == 0) && ((key[1] >= '1') && (key[1] <= '4')))
                {
                        if (strcmp_P(value, PSTR("on")) == 0)
                                time_enable[key[1] - '1'] = true;
                        else
                                time_enable[key[1] - '1'] = false;
                }
                else if ((key[0] == 'z') && (key[2] == 0) && ((key[1] >= 'b') && (key[1] <= ('a' + GetNumZones()))))
                {
                        sched.zone_duration[key[1] - 'b'] = atoi(value);
                }
        }

        // cycle through the time enable bits and set our special code for disabled times:
        for (int i = 0; i < 4; i++)
        {
                if (!time_enable[i])
                        sched.time[i] = -1;
        }

        // Now let's determine what schedule index we are dumping this into.
        int iNumSchedules = GetNumSchedules();
        if (sched_num == -1)
        {
                // Check to see if we've exceeded the number of schedules.
                if (iNumSchedules == MAX_SCHEDULES )
                {
                        trace(F("Too Many Schedules\n"));
                        return false;
                }
                sched_num = iNumSchedules++;
                SetNumSchedules(iNumSchedules);
        }

        // check to see if we've got a valid schedule number
        if ((sched_num < 0) || (sched_num >= iNumSchedules))
        {
                trace(F("Invalid Schedule Number :%d\n"), sched_num);
                return false;
        }
        // and save it
        SaveSchedule(sched_num, &sched);
        return true;
}

bool DeleteSchedule(const KVPairs & key_value_pairs)
{
        int sched_num = -1;
        // Iterate through the kv pairs and update the appropriate structure values.
        for (int i = 0; i < key_value_pairs.num_pairs; i++)
        {
                const char * key = key_value_pairs.keys[i];
                const char * value = key_value_pairs.values[i];
                if (strcmp_P(key, PSTR("id")) == 0)
                {
                        sched_num = atoi(value);
                }
        }

        // Now let's determine what schedule index we are deleting this into.
        const int iNumSchedules = GetNumSchedules();

        // see if we're in the proper range
        if ((sched_num < 0) || (sched_num >= iNumSchedules))
                return false;

        Schedule sched;
        for (int i = sched_num; i < (iNumSchedules - 1); i++)
        {
                LoadSchedule(i + 1, &sched);
                SaveSchedule(i, &sched);
        }
        SetNumSchedules(iNumSchedules - 1);
        return true;
}

bool SetZones(const KVPairs & key_value_pairs)
{
		uint8_t		n_zones = GetNumZones();
		
		FullZone	fullZone;

		for( int zn=0; zn<n_zones; zn++ )
		{
			char  zcode = 'b' + zn;
			LoadZone(zn, &fullZone);
			bool  fzChanged = false;

			for (int i = 0; i < key_value_pairs.num_pairs; i++)
			{
                const char * key = key_value_pairs.keys[i];
                const char * value = key_value_pairs.values[i];
                
				if( (key[0] == 'z') && (key[1] == zcode) )	// zone found in kvpairs
                {
						fzChanged = true;
                    
						if (memcmp(key + 2, "name", 5) == 0)
                                strncpy(fullZone.name, value, sizeof(fullZone.name));
                        else if ((key[2] == 'e') && (key[3] == 0))
                        {
                                if (strcmp_P(value, PSTR("on")) == 0)
                                        fullZone.bEnabled = true;
                                else
                                        fullZone.bEnabled = false;
                        }
                        else if ((key[2] == 'p') && (key[3] == 0))
                        {
                                if (strcmp_P(value, PSTR("on")) == 0)
                                        fullZone.bPump = true;
                                else
                                        fullZone.bPump = false;
                        }
                }
			}
			if( fzChanged ) SaveZone(zn, &fullZone);
		}

        return true;
}

bool SetSettings(const KVPairs & key_value_pairs)
{
        for (int i = 0; i < key_value_pairs.num_pairs; i++)
        {
                const char * key = key_value_pairs.keys[i];
                const char * value = key_value_pairs.values[i];
                if (strcmp_P(key, PSTR("ip")) == 0)
                {
                        SetIP(decodeIP(value));
                }
                else if (strcmp_P(key, PSTR("netmask")) == 0)
                {
                        SetNetmask(decodeIP(value));
                }
                else if (strcmp_P(key, PSTR("gateway")) == 0)
                {
                        SetGateway(decodeIP(value));
                }
                else if (strcmp_P(key, PSTR("wuip")) == 0)
                {
                        SetWUIP(decodeIP(value));
                }
                else if (strcmp_P(key, PSTR("apikey")) == 0)
                {
                        SetApiKey(value);
                }
                else if (strcmp_P(key, PSTR("zip")) == 0)
                {
                        SetZip(strtoul(value, 0, 10));
                }
                else if (strcmp_P(key, PSTR("NTPip")) == 0)
                {
                        SetNTPIP(decodeIP(value));
                }
                else if (strcmp_P(key, PSTR("NTPoffset")) == 0)
                {
                        SetNTPOffset(atoi(value));
						nntpTimeServer.flagCheckTime();

                }
                else if (strcmp_P(key, PSTR("ot")) == 0)
                {
                        SetOT((EOT)atoi(value));
                }
                else if (strcmp_P(key, PSTR("webport")) == 0)
                {
                        SetWebPort(atoi(value));
                }
                else if (strcmp_P(key, PSTR("sadj")) == 0)
                {
                        SetSeasonalAdjust(atoi(value));
                }
                else if (strcmp_P(key, PSTR("pws")) == 0)
                {
                        SetPWS(value);
                }
                else if (strcmp_P(key, PSTR("wutype")) == 0)
                {
                        SetUsePWS(strcmp_P(value, PSTR("pws")) == 0);
                }

        }
        return true;
}


//  Load EEPROM from an INI file
//
//	This operation is performed to rebuild IO topology or change other hardware config.
//	When completed successfully this process will fully populate EEPROM (including signature).
//
//	Returns true if all required settings were loaded from ini file, and false otherwise.
//  Note: When particular setting(s) are missing from ini file, defaults are used instead.
//
void ResetEEPROM()
{
	bool  retcode = true;

	const size_t bufferLen = 128;
	char buffer[bufferLen];			// Temp buffer for ini file processing. Must be big enough to hold one line
	char		tmpb[16];			// worker buffer

	strcpy_P(buffer, PSTR(EEPROM_INI_FILE));	// to avoid wasting space use common buffer for the file name init
	IniFile ini(buffer);

	localUI.lcd_print_line_clear_pgm(PSTR("Resetting EEPROM"), 1);

#ifndef HW_ENABLE_SD
//
// SD is not enabled, so we have to create default config. 
//
	ResetEEPROM_NoSD(DEFAULT_STATION_ID);

#else // HW_ENABLE_SD
	if( !ini.open() )
	{
		trace(F("LoadIniEEPROM - error opening device ini file.\n"));

		retcode = false;
	}

// OK, we successfully opened device ini file. Read config and populate EEPROM.
	trace(F("Loading EEPROM from device ini file.\n"));

// First we need to write signature and zero out various configs (that are not loaded from ini file)

		const char * const sHeader = EEPROM_SHEADER;

        for (int i = 0; i <= 3; i++)			// write current signature
                EEPROM.write(i, sHeader[i]);
        
		SetNumSchedules(0);
		SetEvtMasterFlags(0);
		SetEvtMasterStationID(0);

// Validate ini file to ensure we can successfully read it (check max string length)

		if( !ini.validate(buffer, bufferLen) )
		{
			trace(F("LoadIniEEPROM - ini file failed validation.\n"));

			retcode = false;
		}

// And this is the actual device config load from ini file

		IPAddress ip;

		if( ini.getIPAddress_P(PSTR("Network"), PSTR("IP"), buffer, bufferLen, ip) )
			SetIP(ip);
		else 
		{
			SetIP(IPAddress(10, 0, 1, 36));		// default IP address
			retcode = false;
		}
  
		if( ini.getIPAddress_P(PSTR("Network"), PSTR("Subnet"), buffer, bufferLen, ip) )
			SetNetmask(ip);
		else
		{
			SetNetmask(IPAddress(255, 255, 255, 0));	// default Subnet
			retcode = false;
		}
		
		if( ini.getIPAddress_P(PSTR("Network"), PSTR("Gateway"), buffer, bufferLen, ip) )
			SetGateway(ip);
		else
		{
			SetGateway(IPAddress(10, 0, 1, 1));		// default gateway
			retcode = false;
		}

		uint16_t	u16;
		if( ini.getValue_P(PSTR("Network"), PSTR("WebPort"), buffer, bufferLen, u16) )
			SetWebPort(u16);
		else
		{
			SetWebPort(80);			// default HTTP port
			retcode = false;
		}

		if( ini.getIPAddress_P(PSTR("Network"), PSTR("NTPServer"), buffer, bufferLen, ip) )
			SetNTPIP(ip);
		else
		{
			SetNTPIP(IPAddress(204,9,54,119));	// default NTP server
			retcode = false;
		}

		SetWUIP(INADDR_NONE);		// we don't pre-populate Weather Underground IP address
		SetApiKey("");				// we don't pre-populate API key for WU
        SetPWS("");
        SetUsePWS(false);

		int		i16;
		if( ini.getValue_P(PSTR("System"), PSTR("NTPOffset"), buffer, bufferLen, i16) )
			SetNTPOffset(-8);
		else
		{
			SetNTPOffset(i16);
			retcode = false;
		}

		if( ini.getValue_P(PSTR("System"), PSTR("Zip"), buffer, bufferLen, u16) )
			SetZip(u16);
		else
		{
			SetZip(0);
			retcode = false;
		}

		if( ini.getValue_P(PSTR("System"), PSTR("SeasonalAdj"), buffer, bufferLen, i16) )
			SetSeasonalAdjust(i16);
		else
		{
			SetSeasonalAdjust(100);
			retcode = false;
		}

		SetRunSchedules(false);		// no schedules

		SetOT(OT_NONE);	

// Local channels

		uint16_t	parChannels = 0;
		if( !ini.getValue_P(PSTR("LocalChannels"), PSTR("NumParallel"), buffer, bufferLen, parChannels) )
		{
			parChannels = 0;
		}
		trace(F("LoadIniEEPROM - %d parallel channels\n"), parChannels );
		SetNumIOChannels(parChannels);

		if( parChannels > LOCAL_NUM_DIRECT_CHANNELS ) 
		{
			trace(F("LoadIniEEPROM - specified number of Parallel channels %d too high, truncating it to %d.\n"), parChannels, LOCAL_NUM_DIRECT_CHANNELS );
			parChannels = LOCAL_NUM_DIRECT_CHANNELS;	// basic protection to constrain the maximum number of parallel channels
		}

		if( parChannels != 0 ){

			uint16_t	ioPin;
			uint8_t		zoneToIOMap[LOCAL_NUM_DIRECT_CHANNELS] = PARALLEL_PIN_OUT_MAP;

			for( int i=0; i<parChannels; i++ )
			{
				sprintf_P(tmpb, PSTR("Chan%d"), i+1);	// In ini file channels are numbered from 1
				if( ini.getValue("ParallelIOMap", tmpb, buffer, bufferLen, ioPin) )
					zoneToIOMap[i] = ioPin;
			}

			SaveZoneIOMap( zoneToIOMap );

			if( ini.getValue_P(PSTR("LocalChannels"), PSTR("ParallelPolarity"), buffer, bufferLen, tmpb, sizeof(tmpb)-1) )
			{
				if( strcmp_P(tmpb, PSTR("Negative")) == 0 )
					SetOT(OT_DIRECT_NEG);		
				else if( strcmp_P(tmpb, PSTR("Positive")) == 0 )
					SetOT(OT_DIRECT_POS);		
			}
			else
			{
				SetOT(OT_DIRECT_POS);		
			}
		}
		
		uint16_t	serChannels = 0;
		if( ini.getValue_P(PSTR("LocalChannels"), PSTR("NumSerial"), buffer, bufferLen, serChannels) )
			SetNumOSChannels(serChannels);
		else
		{
			SetNumOSChannels(0);
		}

		if( serChannels != 0 ){

			SrIOMapStruct  srIoMap; 

			if( ini.getValue_P(PSTR("SerialIOMap"), PSTR("SrClkPin"), buffer, bufferLen, u16) )
				srIoMap.SrClkPin = u16;
			else
				srIoMap.SrClkPin = 30;

			if( ini.getValue_P(PSTR("SerialIOMap"), PSTR("SrNoePin"), buffer, bufferLen, u16) )
				srIoMap.SrNoePin = u16;
			else
				srIoMap.SrNoePin = 29;

			if( ini.getValue_P(PSTR("SerialIOMap"), PSTR("SrDatPin"), buffer, bufferLen, u16) )
				srIoMap.SrDatPin = u16;
			else
				srIoMap.SrDatPin = 28;

			if( ini.getValue_P(PSTR("SerialIOMap"), PSTR("SrLatPin"), buffer, bufferLen, u16) )
				srIoMap.SrLatPin = u16;
			else
				srIoMap.SrLatPin = 27;

			SaveSrIOMap(&srIoMap);
		}

// Load Stations info

// First let's zero out stations block

		FullStation  fullStation;
		memset(&fullStation,0,sizeof(fullStation));

		for( uint8_t u=0; u<MAX_STATIONS; u++ )
				SaveStation(u, &fullStation);


		uint16_t	numStations = 0;
		if( ini.getValue_P(PSTR("Stations"), PSTR("NumStations"), buffer, bufferLen, numStations) )
		{
			if( numStations >= MAX_STATIONS )
			{
				trace(F("LoadIniEEPROM - invalid number of Stations %d. Number should be <16\n."), numStations);
				numStations = 0;
			}
		}
		else

#ifdef VERBOSE_TRACE
		trace(F("LoadIniEEPROM - numStations=%d\n"), numStations);
#endif
		if( numStations != 0 )	// We have at least one Station defined in the ini file
		{

			char			keyName[16];
			char			sectionName[16];
			uint16_t		stationID;
			uint16_t		netID;
			uint16_t		numChannels = 0;
			uint16_t		netAddr;

			for( int i=0; i<numStations; i++ )
			{

				sprintf_P(sectionName, PSTR("Station%d"), i+1);	// In ini file channels are numbered from 1
				strcpy_P(keyName, PSTR("StationID"));
#ifdef VERBOSE_TRACE
				trace(F("Reading station %s\n"), sectionName);
				freeMemory();
#endif

				if( !ini.getValue(sectionName, keyName, buffer, bufferLen, stationID) )
				{
					trace(F("LoadIniEEPROM - Cannot get StationID for station %d\n"), i);
					goto skip_Station;
				}

				if( stationID >= MAX_STATIONS )
				{
					trace(F("LoadIniEEPROM - invalid StationID %d. Number should be <16\n"), stationID);
					goto skip_Station;
				}

				strcpy_P(keyName, PSTR("NumChannels"));
				if( !ini.getValue(sectionName, keyName, buffer, bufferLen, numChannels) )
				{
					trace(F("LoadIniEEPROM - cannot read NumChannels for Station %d\n"), i);
					goto skip_Station;
				}
				if( numChannels > 8 )
				{
					trace(F("LoadIniEEPROM - NumChannels too high for Station %d, truncating to 8\n"), i);
					numChannels = 8;
				}

				strcpy_P(keyName, PSTR("NetworkID"));
				if( !ini.getValue(sectionName, keyName, buffer, bufferLen, tmpb, sizeof(tmpb)-1) )
				{
					trace(F("LoadIniEEPROM - cannot read NetworkID for Station %d, error code: %d\n"), i, ini.getError());
					goto skip_Station;
				}
				if( strcmp_P(tmpb, PSTR("Parallel")) == 0 )
					netID = NETWORK_ID_LOCAL_PARALLEL;
				else if( strcmp_P(tmpb, PSTR("Serial")) == 0 )
					netID = NETWORK_ID_LOCAL_SERIAL;
				else if( strcmp_P(tmpb, PSTR("XBee")) == 0 )
					netID = NETWORK_ID_XBEE;
				else
				{
					trace(F("NetworkID not recognized for station %d, skipping the station\n"), i+1);
					goto skip_Station;
				}
#ifdef VERBOSE_TRACE
				trace(F("Got NetworkID of %s (code %d) for Station %d\n"), tmpb, netID, i+1);
#endif
				strcpy_P(keyName, PSTR("NetworkAddress"));
				if( !ini.getValue(sectionName, keyName, buffer, bufferLen, netAddr) )
				{
					trace(F("LoadIniEEPROM - cannot read NetworkAddress for Station %d\n"), i);
					goto skip_Station;
				}

				memset(&fullStation,0,sizeof(fullStation));
				fullStation.stationFlags = STATION_FLAGS_VALID | STATION_FLAGS_ENABLED;
				fullStation.networkID = netID;
				fullStation.networkAddress = netAddr;
				fullStation.numZoneChannels = numChannels;

				sprintf_P(fullStation.name, PSTR("Station %d"), stationID);

				SaveStation(stationID, &fullStation);	// save the station

				trace(F("LoadIniEEPROM - Saving station %d, NumChannels %d, netID %d, netAddr %d\n"), (int)stationID, (int)numChannels, (int)netID, (int)netAddr);
skip_Station:;
			}
		}

		if( ini.getValue_P(PSTR("LocalChannels"), PSTR("ParallelPolarity"), buffer, bufferLen, tmpb, sizeof(tmpb)-1) )
		{
			if( strcmp_P(tmpb, PSTR("Negative")) == 0 )
				SetOT(OT_DIRECT_NEG);		
			else if( strcmp_P(tmpb, PSTR("Positive")) == 0 )
				SetOT(OT_DIRECT_POS);		
		}

		// Sensors definitions
		uint16_t	numSensors = 0;
		SetNumSensors(0);	// initial default

		if( ini.getValue_P(PSTR("Sensors"), PSTR("NumSensors"), buffer, bufferLen, numSensors) )
		{
			if( numSensors >= MAX_SENSORS )
			{
				trace(F("LoadIniEEPROM - invalid number of Sensors %d. Number should be <%d\n."), numSensors, (int)MAX_SENSORS);
				numSensors = 0;
			}
		}


#ifdef VERBOSE_TRACE
		trace(F("numSensors=%d\n"), numSensors);
#endif

		if( numSensors != 0 )	// We have at least one Sensor defined in the ini file
		{

			char			keyName[16];
			char			sectionName[16];
			uint8_t			sensType;
			uint8_t			sensID = 0;
			uint16_t			sensStation;
			uint16_t			sensChannel;

			for( int i=1; i<=numSensors; i++ )
			{

				sprintf_P(sectionName, PSTR("Sensor%d"), i);	
				strcpy_P(keyName, PSTR("Type"));
				if( !ini.getValue(sectionName, keyName, buffer, bufferLen, tmpb, sizeof(tmpb)-1) )
				{
					trace(F("LoadIniEEPROM - Cannot get Type for Sensor%d\n"), i);
					goto skip_Sensor;
				}
#ifdef VERBOSE_TRACE
				trace(F("Reading Sensor%d, type=%s\n"), i, tmpb);
#endif

				if( strcmp_P(tmpb, PSTR("Temperature")) == 0 )
					sensType = SENSOR_TYPE_TEMPERATURE;
				else if( strcmp_P(tmpb, PSTR("Pressure")) == 0 )
					sensType = SENSOR_TYPE_PRESSURE;
				else if( strcmp_P(tmpb, PSTR("Humidity")) == 0 )
					sensType = SENSOR_TYPE_HUMIDITY;
				else if( strcmp_P(tmpb, PSTR("Waterflow")) == 0 )
					sensType = SENSOR_TYPE_WATERFLOW;
				else if( strcmp_P(tmpb, PSTR("Voltage")) == 0 )
					sensType = SENSOR_TYPE_VOLTAGE;
				else
				{
					trace(F("Sensor%d type not recognized - skipping it\n"), i);
					goto skip_Sensor;
				}

				strcpy_P(keyName, PSTR("Station"));
				if( !ini.getValue(sectionName, keyName, buffer, bufferLen, sensStation) )
				{
					trace(F("LoadIniEEPROM - cannot read Station for Sensor %d\n"), i);
					goto skip_Sensor;
				}
				if( sensStation > MAX_STATIONS )
				{
					trace(F("LoadIniEEPROM - Sensor Station too high for Sensor%d\n"), i);
					goto skip_Sensor;
				}

				strcpy_P(keyName, PSTR("Channel"));
				if( !ini.getValue(sectionName, keyName, buffer, bufferLen, sensChannel) )
				{
					trace(F("LoadIniEEPROM - cannot read Channel for Sensor%d\n"), i);
					goto skip_Sensor;
				}
				if( sensChannel > 254 )
				{
					trace(F("LoadIniEEPROM - Channel is too high for Sensor%d\n"), i);
					goto skip_Sensor;
				}

				strcpy_P(keyName, PSTR("Name"));
				if( !ini.getValue(sectionName, keyName, buffer, bufferLen, tmpb, sizeof(tmpb)-1) )
				{
					trace(F("LoadIniEEPROM - Cannot get Name for Sensor%d\n"), i);
					sprintf_P(tmpb, PSTR("Sensor %u:%u"), sensStation, sensChannel);	// if no name provided - generate default
				}

				{
					FullSensor  fullSens;

					fullSens.sensorType = sensType;
					fullSens.sensorChannel = sensChannel;
					fullSens.sensorStationID = sensStation;
					fullSens.flags = 0;	
					memcpy(fullSens.name, tmpb, 20);	// sensor name is limited to 20 ASCII chars, and we just copy the block. If the actual name is shorter, it will have null in it.

					SaveSensor(sensID, &fullSens);	// save the sensor

					trace(F("LoadIniEEPROM - Saving sensor %d\n"), (int)sensID);

					sensID++;
				}
skip_Sensor:;
			}
			SetNumSensors(sensID);
#ifdef VERBOSE_TRACE
			trace(F("LoadIniEEPROM - saved %d sensors\n"), sensID);
#endif
		}

// Now let's iterate through Stations and fill in Zones list

		FullZone zone = {0};
		uint8_t	 zoneIndex = 0;

		for( int st=0; st<MAX_STATIONS; st++ )
		{
			LoadStation(st,&fullStation);

			if( (fullStation.stationFlags & STATION_FLAGS_VALID) && (fullStation.stationFlags & STATION_FLAGS_ENABLED) )
			{
				for( uint8_t j=0; j<fullStation.numZoneChannels; j++ )
				{
						if( j == 0 ){

							fullStation.startZone = zoneIndex;			// save starting zone# back reference
							SaveStation(st, &fullStation);		// in Station entity
						}

						zone.bEnabled = 1;
						zone.bPump = false;
						zone.stationID = st;
						zone.channel = j;
						sprintf_P(zone.name, PSTR("Zone %d, Loc %X:%d"), zoneIndex + 1, zone.stationID, zone.channel+1);
						
//						trace(F("Created zone %d, \"%s\"\n"), (uint8_t)(zoneIndex + 1), zone.name);
						
						SaveZone(zoneIndex, &zone);
						zoneIndex++;
				}
			}
		}

		SetNumZones(zoneIndex);
		trace(F("LoadIniEEPROM - Generated %d Zones\n"), zoneIndex);

// Load XBee config

		SetXBeeFlags(0);	// XBee disabled by default
		if( ini.getValue_P(PSTR("XBee"), PSTR("Enabled"), buffer, bufferLen, tmpb, sizeof(tmpb)-1) )
		{
			if( !strcmp_P(tmpb, PSTR("Yes")) || !strcmp_P(tmpb, PSTR("yes")) ){
// XBee enabled
				SetXBeeFlags(NETWORK_FLAGS_ENABLED);
					
				if( ini.getValue_P(PSTR("XBee"), PSTR("Port"), buffer, bufferLen, u16 ) )
					SetXBeePort(u16);
				else
					SetXBeePort(NETWORK_XBEE_DEFAULT_PORT);

				if( ini.getValue_P(PSTR("XBee"), PSTR("Speed"), buffer, bufferLen, u16 ) )
					SetXBeePortSpeed(u16);
				else
					SetXBeePortSpeed(NETWORK_XBEE_DEFAULT_SPEED);

				if( ini.getValue_P(PSTR("XBee"), PSTR("PANID"), buffer, bufferLen, u16 ) )
					SetXBeePANID(u16);
				else
					SetXBeePANID(NETWORK_XBEE_DEFAULT_PANID);

				if( ini.getValue_P(PSTR("XBee"), PSTR("Channel"), buffer, bufferLen, u16 ) )
					SetXBeeChan(u16);
				else
					SetXBeeChan(NETWORK_XBEE_DEFAULT_CHAN);
			}
		}

		localUI.lcd_print_line_clear_pgm(PSTR("EEPROM reloaded"), 0);
		localUI.lcd_print_line_clear_pgm(PSTR("Rebooting..."), 1);
		delay(2000);

		sysreset();
#endif // HW_ENABLE_SD

}

void 	ResetEEPROM_NoSD(uint8_t  defStationID)
{
#ifndef HW_ENABLE_SD
		trace(F("Loading EEPROM using default config (no device.ini file).\n"));

// First we need to write signature and zero out various configs (that are not loaded from ini file)

		const char * const sHeader = EEPROM_SHEADER;

        for (int i = 0; i <= 3; i++)				// write current signature
                EEPROM.write(i, sHeader[i]);
        
		SetNumSchedules(0);
		SetEvtMasterFlags(0);
		SetEvtMasterStationID(0);

		SetIP(IPAddress(10, 0, 1, 36));				// default IP address  
		SetNetmask(IPAddress(255, 255, 255, 0));	// default Subnet
		SetGateway(IPAddress(10, 0, 1, 1));			// default gateway
		SetWebPort(80);								// default HTTP port
		SetNTPIP(IPAddress(204,9,54,119));			// default NTP server

		SetWUIP(INADDR_NONE);						// we don't pre-populate Weather Underground IP address
		SetApiKey("");								// we don't pre-populate API key for WU
        SetPWS("");
        SetUsePWS(false);

		SetNTPOffset(-8);
		SetZip(0);
		SetSeasonalAdjust(100);
		SetRunSchedules(false);		// no schedules
		SetOT(OT_NONE);	

// Local channels

		uint16_t	parChannels = LOCAL_NUM_DIRECT_CHANNELS;

		trace(F("LoadIniEEPROM - %d parallel channels\n"), parChannels );
		SetNumIOChannels(parChannels);

		uint8_t		zoneToIOMap[LOCAL_NUM_DIRECT_CHANNELS] = PARALLEL_PIN_OUT_MAP;
		SaveZoneIOMap( zoneToIOMap );
		SetOT(OT_DIRECT_POS);		
		
		SetNumOSChannels(0);

// Load Stations info
// First let's zero out stations block

		FullStation  fullStation;
		memset(&fullStation,0,sizeof(fullStation));

		for( uint8_t u=0; u<MAX_STATIONS; u++ )
				SaveStation(u, &fullStation);

		uint16_t	numStations = 1;
		memset(&fullStation,0,sizeof(fullStation));
		fullStation.stationFlags = STATION_FLAGS_VALID | STATION_FLAGS_ENABLED | STATION_FLAGS_RSTATUS | STATION_FLAGS_RCONTROL;
		fullStation.networkID = DEFAULT_STATION_NETWORK_TYPE;
		fullStation.networkAddress = DEFAULT_STATION_NETWORK_ADDRESS;
		fullStation.numZoneChannels = DEFAULT_STATION_NUM_ZONES;

		sprintf_P(fullStation.name, PSTR("Default Station"));

		SaveStation(defStationID, &fullStation);
		SetMyStationID(defStationID);

		trace(F("Creating default station\n"));

		// Sensors definitions
		SetNumSensors(0);	// initial default

		{
			uint8_t			sensID = 0;

#ifdef SENSOR_ENABLE_DHT
			{
				FullSensor  fullSens;

				fullSens.sensorType = SENSOR_TYPE_TEMPERATURE;
				fullSens.sensorChannel = SENSOR_CHANNEL_DHT_TEMPERATURE;
				fullSens.sensorStationID = DEFAULT_STATION_ID;
				fullSens.flags = 0;	
				sprintf_P(fullSens.name, PSTR("Sensor %u:%u"), DEFAULT_STATION_ID, SENSOR_CHANNEL_DHT_TEMPERATURE);	

				SaveSensor(sensID, &fullSens);	// save the sensor
				sensID++;

				fullSens.sensorType = SENSOR_TYPE_HUMIDITY;
				fullSens.sensorChannel = SENSOR_CHANNEL_DHT_HUMIDITY;
				fullSens.sensorStationID = DEFAULT_STATION_ID;
				fullSens.flags = 0;	
				sprintf_P(fullSens.name, PSTR("Sensor %u:%u"), DEFAULT_STATION_ID, SENSOR_CHANNEL_DHT_HUMIDITY);	

				SaveSensor(sensID, &fullSens);	// save the sensor
				sensID++;
			}
#endif // SENSOR_ENABLE_DHT

#ifdef SENSOR_ENABLE_BMP180
			{
				FullSensor  fullSens;

				fullSens.sensorType = SENSOR_TYPE_TEMPERATURE;
				fullSens.sensorChannel = SENSOR_CHANNEL_BMP180_TEMPERATURE;
				fullSens.sensorStationID = DEFAULT_STATION_ID;
				fullSens.flags = 0;	
				sprintf_P(fullSens.name, PSTR("Sensor %u:%u"), DEFAULT_STATION_ID, SENSOR_CHANNEL_BMP180_TEMPERATURE);	

				SaveSensor(sensID, &fullSens);	// save the sensor
				sensID++;

				fullSens.sensorType = SENSOR_TYPE_PRESSURE;
				fullSens.sensorChannel = SENSOR_CHANNEL_BMP180_PRESSURE;
				fullSens.sensorStationID = DEFAULT_STATION_ID;
				fullSens.flags = 0;	
				sprintf_P(fullSens.name, PSTR("Sensor %u:%u"), DEFAULT_STATION_ID, SENSOR_CHANNEL_BMP180_PRESSURE);	

				SaveSensor(sensID, &fullSens);	// save the sensor
				sensID++;
			}
#endif // SENSOR_ENABLE_BMP180

			SetNumSensors(sensID);
			trace(F("LoadIniEEPROM - saved %d sensors\n"), sensID);
		}

// Now let's iterate through Stations and fill in Zones list

		FullZone zone = {0};
		uint8_t	 zoneIndex = 0;

		for( int st=0; st<MAX_STATIONS; st++ )
		{
			LoadStation(st,&fullStation);

			if( (fullStation.stationFlags & STATION_FLAGS_VALID) && (fullStation.stationFlags & STATION_FLAGS_ENABLED) )
			{
				for( uint8_t j=0; j<fullStation.numZoneChannels; j++ )
				{
						if( j == 0 ){

							fullStation.startZone = zoneIndex;			// save starting zone# back reference
							SaveStation(st, &fullStation);		// in Station entity
						}

						zone.bEnabled = 1;
						zone.bPump = false;
						zone.stationID = st;
						zone.channel = j;
						sprintf_P(zone.name, PSTR("Zone %d, Loc %X:%d"), zoneIndex + 1, zone.stationID, zone.channel+1);
						
//						trace(F("Created zone %d, \"%s\"\n"), (uint8_t)(zoneIndex + 1), zone.name);
						
						SaveZone(zoneIndex, &zone);
						zoneIndex++;
				}
			}
		}

		SetNumZones(zoneIndex);
		trace(F("LoadIniEEPROM - Generated %d Zones\n"), zoneIndex);

// Load XBee config

// XBee enabled by default
		SetXBeeFlags(NETWORK_FLAGS_ENABLED);
		SetXBeePort(NETWORK_XBEE_DEFAULT_PORT);
		SetXBeePortSpeed(NETWORK_XBEE_DEFAULT_SPEED);
		SetXBeePANID(NETWORK_XBEE_DEFAULT_PANID);
		SetXBeeChan(NETWORK_XBEE_DEFAULT_CHAN);

// show message and reboot
		localUI.lcd_print_line_clear_pgm(PSTR("EEPROM reloaded"), 0);
		localUI.lcd_print_line_clear_pgm(PSTR("Rebooting..."), 1);
		delay(2000);

		sysreset();

#endif // HW_ENABLE_SD
}


void SetNumSchedules(const uint8_t iNum)
{
        EEPROM.write(ADDR_SCHEDULE_COUNT, iNum);
}

uint8_t GetNumSchedules()
{
        return EEPROM.read(ADDR_SCHEDULE_COUNT);
}

void SetNTPOffset(const int8_t value)
{
        EEPROM.write(ADDR_NTP_OFFSET, value);
}

int8_t GetNTPOffset()
{
        return EEPROM.read(ADDR_NTP_OFFSET);
}

IPAddress GetNTPIP()
{
        return IPAddress(EEPROM.read(ADDR_NTP_IP), EEPROM.read(ADDR_NTP_IP + 1), EEPROM.read(ADDR_NTP_IP + 2), EEPROM.read(ADDR_NTP_IP + 3));
}

void SetNTPIP(const IPAddress & value)
{
        for (int i = 0; i < 4; i++)
                EEPROM.write(ADDR_NTP_IP + i, value[i]);
}

IPAddress GetIP()
{
        return IPAddress(EEPROM.read(ADDR_IP), EEPROM.read(ADDR_IP + 1), EEPROM.read(ADDR_IP + 2), EEPROM.read(ADDR_IP + 3));
}

void SetIP(const IPAddress & value)
{
        for (int i = 0; i < 4; i++)
                EEPROM.write(ADDR_IP + i, value[i]);
}

IPAddress GetNetmask()
{
        return IPAddress(EEPROM.read(ADDR_NETMASK), EEPROM.read(ADDR_NETMASK + 1), EEPROM.read(ADDR_NETMASK + 2), EEPROM.read(ADDR_NETMASK + 3));
}

void SetNetmask(const IPAddress & value)
{
        for (int i = 0; i < 4; i++)
                EEPROM.write(ADDR_NETMASK + i, value[i]);
}

IPAddress GetGateway()
{
        return IPAddress(EEPROM.read(ADDR_GATEWAY), EEPROM.read(ADDR_GATEWAY + 1), EEPROM.read(ADDR_GATEWAY + 2), EEPROM.read(ADDR_GATEWAY + 3));
}

void SetGateway(const IPAddress & value)
{
        for (int i = 0; i < 4; i++)
                EEPROM.write(ADDR_GATEWAY + i, value[i]);
}

IPAddress GetWUIP()
{
        return IPAddress(EEPROM.read(ADDR_WUIP), EEPROM.read(ADDR_WUIP + 1), EEPROM.read(ADDR_WUIP + 2), EEPROM.read(ADDR_WUIP + 3));
}

void SetWUIP(const IPAddress & value)
{
        for (int i = 0; i < 4; i++)
                EEPROM.write(ADDR_WUIP + i, value[i]);
}

uint32_t GetZip()
{
        return (uint32_t) EEPROM.read(ADDR_ZIP) << 24 | (uint32_t) EEPROM.read(ADDR_ZIP + 1) << 16 | (uint32_t) EEPROM.read(ADDR_ZIP + 2) << 8
                        | (uint32_t) EEPROM.read(ADDR_ZIP + 3);
}

void SetZip(const uint32_t zip)
{
        for (int i = 0; i < 4; i++)
                EEPROM.write(ADDR_ZIP + i, zip >> (8 * (3 - i)));
}

void GetPWS(char * key)
{
        for (int i=0; i<11; i++)
                key[i] = EEPROM.read(ADDR_PWS+i);
}

void SetPWS(const char * key)
{
        for (int i=0; i<11; i++)
                EEPROM.write(ADDR_PWS+i, key[i]);
}

void GetApiKey(char * key)
{
        sprintf_P(key, PSTR("%02x%02x%02x%02x%02x%02x%02x%02x"), EEPROM.read(ADDR_APIKEY), EEPROM.read(ADDR_APIKEY + 1), EEPROM.read(ADDR_APIKEY + 2),
                        EEPROM.read(ADDR_APIKEY + 3), EEPROM.read(ADDR_APIKEY + 4), EEPROM.read(ADDR_APIKEY + 5), EEPROM.read(ADDR_APIKEY + 6),
                        EEPROM.read(ADDR_APIKEY + 7));
}

static uint8_t toHex(char val)
{
        if ((val >= '0') && (val <= '9'))
                return val - '0';
        else if ((val >= 'A') && (val <= 'F'))
                return val - 'A' + 10;
        else if ((val >= 'a') && (val <= 'f'))
                return val - 'a' + 10;
        else
                return 0;
}

void SetApiKey(const char * key)
{
        if (strlen(key) != 16)
        {
                for (int i = 0; i < 8; i++)
                        EEPROM.write(ADDR_APIKEY + i, 0);
        }
        else
        {
                for (int i = 0; i < 8; i++)
                {
                        EEPROM.write(ADDR_APIKEY + i, (toHex(key[i * 2]) << 4) | toHex(key[i * 2 + 1]));
                }
        }
}

bool GetRunSchedules()
{
        return EEPROM.read(ADDR_OP1) & 0x01;
}

void SetRunSchedules(bool value)
{
        uint8_t current = EEPROM.read(ADDR_OP1);
        if (value)
                EEPROM.write(ADDR_OP1, current | 0x01);
        else
                EEPROM.write(ADDR_OP1, current & ~0x01);
}

bool GetUsePWS()
{
        return EEPROM.read(ADDR_OP1) & 0x02;
}

void SetUsePWS(bool value)
{
        uint8_t current = EEPROM.read(ADDR_OP1);
        if (value)
                EEPROM.write(ADDR_OP1, current | 0x02);
        else
                EEPROM.write(ADDR_OP1, current & ~0x02);
}

bool GetDHCP()
{
        return EEPROM.read(ADDR_DHCP);
}

void SetDHCP(const bool value)
{
        EEPROM.write(ADDR_DHCP, value);
}

EOT GetOT()
{
        return (EOT)EEPROM.read(ADDR_OTYPE);
}

void SetOT(EOT oType)
{
        // if things have changed make sure we re-run the io_setup routine.
        if (GetOT() != oType)
        {
                EEPROM.write(ADDR_OTYPE, oType);
                lBoardParallel.begin();
                lBoardSerial.begin();
        }
}

uint16_t GetWebPort()
{
        return EEPROM.read(ADDR_WEB)<<8 | EEPROM.read(ADDR_WEB+1);
}

void SetWebPort(uint16_t port)
{
        EEPROM.write(ADDR_WEB, port>>8);
        EEPROM.write(ADDR_WEB+1, port&0x00FF);
}

uint8_t GetSeasonalAdjust()
{
        return EEPROM.read(ADDR_SADJ);
}

void SetSeasonalAdjust(uint8_t val)
{
        EEPROM.write(ADDR_SADJ, min(val, 200));
}

bool IsFirstBoot()
{
		const char * const sHeader = EEPROM_SHEADER;

        if ((SCHEDULE_INDEX < sizeof(Schedule)) || (ZONE_INDEX < sizeof(FullZone)))
        {
                trace (F("Size mismatch."));
                exit(1);
        }

        if ((EEPROM.read(0) == sHeader[0]) && (EEPROM.read(1) == sHeader[1]) && (EEPROM.read(2) == sHeader[2]) && (EEPROM.read(3) == sHeader[3]))
                return false;
        return true;
}

int GetNumEnabledZones()
{
        ShortZone sz;
        int retval = 0;
        for (int i=0; i<GetNumZones(); i++)
        {
                LoadShortZone(i, &sz);
                if (sz.bEnabled)
                        retval++;
        }
        return retval;
}

uint8_t GetNumSensors(void)
{
	return EEPROM.read(ADDR_NUM_SENSORS);
}

void SetNumSensors(uint8_t numSensors)
{
	EEPROM.write(ADDR_NUM_SENSORS, numSensors);
}

// get number of valid and enabled stations
uint8_t GetNumStations(void)
{
	ShortStation sStation;
	uint8_t		 numS = 0;

	for( uint8_t i=0; i<MAX_STATIONS; i++ )
	{
		LoadShortStation(i, &sStation);

		if( (sStation.stationFlags & STATION_FLAGS_VALID) && (sStation.stationFlags & STATION_FLAGS_ENABLED) )
			numS++;
	}

	return numS;
}

uint16_t GetEvtMasterFlags(void)
{
	uint16_t flags;

	flags = EEPROM.read(ADDR_EVTMASTER_FLAGS+1) << 8;
	flags += EEPROM.read(ADDR_EVTMASTER_FLAGS);

	return flags;
}

uint8_t  GetEvtMasterStationID(void)
{
	return EEPROM.read(ADDR_EVTMASTER_STATIONID);
}

void SetEvtMasterFlags(uint16_t flags)
{
	uint8_t flagsH = flags >> 8;
	uint8_t flagsL = flags & 0x0FF;

	EEPROM.write(ADDR_EVTMASTER_FLAGS+1, flagsH);
	EEPROM.write(ADDR_EVTMASTER_FLAGS, flagsL);
}

void SetEvtMasterStationID(uint8_t stationID)
{
	EEPROM.write(ADDR_EVTMASTER_STATIONID, stationID);
}



Schedule quickSchedule;

