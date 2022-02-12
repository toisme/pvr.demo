/*
 *  Copyright (C) 2011-2021 Team Kodi (https://kodi.tv)
 *  Copyright (C) 2011 Pulse-Eight (http://www.pulse-eight.com/)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#include "PVRDemo.h"

#include <algorithm>
#include <kodi/General.h>
#include <kodi/Filesystem.h>
#include <json/json.h>

/***********************************************************
  * PVR Client AddOn specific public library functions
  ***********************************************************/

CPVRDemo::CPVRDemo()
{
  m_iEpgStart = -1;
  m_strDefaultIcon = "http://www.royalty-free.tv/news/wp-content/uploads/2011/06/cc-logo1.jpg";
  m_strDefaultMovie = "";

  LoadDemoData();

  AddMenuHook(kodi::addon::PVRMenuhook(1, 30000, PVR_MENUHOOK_SETTING));
  AddMenuHook(kodi::addon::PVRMenuhook(2, 30001, PVR_MENUHOOK_ALL));
  AddMenuHook(kodi::addon::PVRMenuhook(3, 30002, PVR_MENUHOOK_CHANNEL));
}

CPVRDemo::~CPVRDemo()
{
  m_channels.clear();
  m_groups.clear();
}

PVR_ERROR CPVRDemo::GetCapabilities(kodi::addon::PVRCapabilities& capabilities)
{
  capabilities.SetSupportsEPG(true);
  capabilities.SetSupportsTV(true);
  capabilities.SetSupportsRadio(true);
  capabilities.SetSupportsChannelGroups(true);
  capabilities.SetSupportsRecordings(true);
  capabilities.SetSupportsRecordingsUndelete(true);
  capabilities.SetSupportsTimers(true);
  capabilities.SetSupportsRecordingsRename(false);
  capabilities.SetSupportsRecordingsLifetimeChange(false);
  capabilities.SetSupportsDescrambleInfo(false);

  return PVR_ERROR_NO_ERROR;
}

PVR_ERROR CPVRDemo::GetBackendName(std::string& name)
{
  name = "pulse-eight demo pvr add-on";
  return PVR_ERROR_NO_ERROR;
}

PVR_ERROR CPVRDemo::GetBackendVersion(std::string& version)
{
  version = "0.1";
  return PVR_ERROR_NO_ERROR;
}

PVR_ERROR CPVRDemo::GetConnectionString(std::string& connection)
{
  connection = "connected";
  return PVR_ERROR_NO_ERROR;
}

PVR_ERROR CPVRDemo::GetBackendHostname(std::string& hostname)
{
  hostname = "";
  return PVR_ERROR_NO_ERROR;
}

PVR_ERROR CPVRDemo::GetDriveSpace(uint64_t& total, uint64_t& used)
{
  total = 1024 * 1024 * 1024;
  used = 0;
  return PVR_ERROR_NO_ERROR;
}

PVR_ERROR CPVRDemo::GetEPGForChannel(int channelUid,
                                     time_t start,
                                     time_t end,
                                     kodi::addon::PVREPGTagsResultSet& results)
{
  if (m_iEpgStart == -1)
    m_iEpgStart = start;

  time_t iLastEndTime = m_iEpgStart + 1;
  int iAddBroadcastId = 0;

  for (auto& myChannel : m_channels)
  {
    if (myChannel.iUniqueId != channelUid)
      continue;

    while (iLastEndTime < end && myChannel.epg.size() > 0)
    {
      time_t iLastEndTimeTmp = 0;
      for (unsigned int iEntryPtr = 0; iEntryPtr < myChannel.epg.size(); iEntryPtr++)
      {
        PVRDemoEpgEntry& myTag = myChannel.epg.at(iEntryPtr);

        kodi::addon::PVREPGTag tag;
        tag.SetUniqueBroadcastId(myTag.iBroadcastId + iAddBroadcastId);
        tag.SetUniqueChannelId(channelUid);
        tag.SetTitle(myTag.strTitle);
        tag.SetStartTime(myTag.startTime + iLastEndTime);
        tag.SetEndTime(myTag.endTime + iLastEndTime);
        tag.SetPlotOutline(myTag.strPlotOutline);
        tag.SetPlot(myTag.strPlot);
        tag.SetIconPath(myTag.strIconPath);
        tag.SetGenreType(myTag.iGenreType);
        tag.SetGenreSubType(myTag.iGenreSubType);
        tag.SetFlags(EPG_TAG_FLAG_UNDEFINED);
        tag.SetSeriesNumber(myTag.iSeriesNumber);
        tag.SetEpisodeNumber(myTag.iEpisodeNumber);
        tag.SetEpisodeName(myTag.strEpisodeName);

        iLastEndTimeTmp = tag.GetEndTime();

        results.Add(tag);
      }

      iLastEndTime = iLastEndTimeTmp;
      iAddBroadcastId += myChannel.epg.size();
    }
  }

  return PVR_ERROR_NO_ERROR;
}

PVR_ERROR CPVRDemo::IsEPGTagPlayable(const kodi::addon::PVREPGTag& tag, bool& bIsPlayable)
{
  bIsPlayable = false;

  for (const auto& channel : m_channels)
  {
    if (channel.iUniqueId == tag.GetUniqueChannelId())
    {
      bIsPlayable = channel.bArchive;
    }
  }

  return PVR_ERROR_NO_ERROR;
}

PVR_ERROR CPVRDemo::GetEPGTagStreamProperties(
    const kodi::addon::PVREPGTag& tag, std::vector<kodi::addon::PVRStreamProperty>& properties)
{
  properties.emplace_back(
      PVR_STREAM_PROPERTY_STREAMURL,
      "https://mirrors.kodi.tv/demo-files/BBB/bbb_sunflower_1080p_30fps_normal.mp4");
  return PVR_ERROR_NO_ERROR;
}

PVR_ERROR CPVRDemo::GetChannelsAmount(int& amount)
{
  amount = m_channels.size();
  return PVR_ERROR_NO_ERROR;
}

PVR_ERROR CPVRDemo::GetChannels(bool bRadio, kodi::addon::PVRChannelsResultSet& results)
{
  for (const auto& channel : m_channels)
  {
    if (channel.bRadio == bRadio)
    {
      kodi::addon::PVRChannel kodiChannel;

      kodiChannel.SetUniqueId(channel.iUniqueId);
      kodiChannel.SetIsRadio(channel.bRadio);
      kodiChannel.SetChannelNumber(channel.iChannelNumber);
      kodiChannel.SetSubChannelNumber(channel.iSubChannelNumber);
      kodiChannel.SetChannelName(channel.strChannelName);
      kodiChannel.SetEncryptionSystem(channel.iEncryptionSystem);
      kodiChannel.SetIconPath(channel.strIconPath);
      kodiChannel.SetIsHidden(false);
      kodiChannel.SetHasArchive(channel.bArchive);

      results.Add(kodiChannel);
    }
  }

  return PVR_ERROR_NO_ERROR;
}

PVR_ERROR CPVRDemo::GetChannelStreamProperties(
    const kodi::addon::PVRChannel& channel, std::vector<kodi::addon::PVRStreamProperty>& properties)
{
  PVRDemoChannel addonChannel;
  GetChannel(channel, addonChannel);

  properties.emplace_back(PVR_STREAM_PROPERTY_STREAMURL, addonChannel.strStreamURL);
  properties.emplace_back(PVR_STREAM_PROPERTY_ISREALTIMESTREAM, "true");
  return PVR_ERROR_NO_ERROR;
}

PVR_ERROR CPVRDemo::GetChannelGroupsAmount(int& amount)
{
  amount = m_groups.size();
  return PVR_ERROR_NO_ERROR;
}

PVR_ERROR CPVRDemo::GetChannelGroups(bool bRadio, kodi::addon::PVRChannelGroupsResultSet& results)
{
  for (const auto& group : m_groups)
  {
    if (group.bRadio == bRadio)
    {
      kodi::addon::PVRChannelGroup kodiGroup;

      kodiGroup.SetIsRadio(bRadio);
      kodiGroup.SetPosition(group.iPosition);
      kodiGroup.SetGroupName(group.strGroupName);
      results.Add(kodiGroup);
    }
  }

  return PVR_ERROR_NO_ERROR;
}

PVR_ERROR CPVRDemo::GetChannelGroupMembers(const kodi::addon::PVRChannelGroup& group,
                                           kodi::addon::PVRChannelGroupMembersResultSet& results)
{
  for (const auto& myGroup : m_groups)
  {
    if (myGroup.strGroupName == group.GetGroupName())
    {
      for (int iId : myGroup.members)
      {
        if (iId < 1 || iId > static_cast<int>(m_channels.size()))
        {
          kodi::Log(ADDON_LOG_ERROR, "ignoring invalid channel id '%d')", iId);
          continue;
        }

        PVRDemoChannel& channel = m_channels.at(iId - 1);
        kodi::addon::PVRChannelGroupMember kodiGroupMember;
        kodiGroupMember.SetGroupName(group.GetGroupName());
        kodiGroupMember.SetChannelUniqueId(channel.iUniqueId);
        kodiGroupMember.SetChannelNumber(channel.iChannelNumber);
        kodiGroupMember.SetSubChannelNumber(channel.iSubChannelNumber);

        results.Add(kodiGroupMember);
      }
    }
  }

  return PVR_ERROR_NO_ERROR;
}

PVR_ERROR CPVRDemo::GetSignalStatus(int channelUid, kodi::addon::PVRSignalStatus& signalStatus)
{
  signalStatus.SetAdapterName("pvr demo adapter 1");
  signalStatus.SetAdapterStatus("OK");

  return PVR_ERROR_NO_ERROR;
}

PVR_ERROR CPVRDemo::GetRecordingsAmount(bool deleted, int& amount)
{
  amount = deleted ? m_recordingsDeleted.size() : m_recordings.size();
  return PVR_ERROR_NO_ERROR;
}

PVR_ERROR CPVRDemo::GetRecordings(bool deleted, kodi::addon::PVRRecordingsResultSet& results)
{
  for (const auto& recording : deleted ? m_recordingsDeleted : m_recordings)
  {
    kodi::addon::PVRRecording kodiRecording;

    kodiRecording.SetDuration(recording.iDuration);
    kodiRecording.SetGenreType(recording.iGenreType);
    kodiRecording.SetGenreSubType(recording.iGenreSubType);
    kodiRecording.SetRecordingTime(recording.recordingTime);
    kodiRecording.SetEpisodeNumber(recording.iEpisodeNumber);
    kodiRecording.SetSeriesNumber(recording.iSeriesNumber);
    kodiRecording.SetIsDeleted(deleted);
    kodiRecording.SetChannelType(recording.bRadio ? PVR_RECORDING_CHANNEL_TYPE_RADIO
                                                  : PVR_RECORDING_CHANNEL_TYPE_TV);
    kodiRecording.SetChannelName(recording.strChannelName);
    kodiRecording.SetPlotOutline(recording.strPlotOutline);
    kodiRecording.SetPlot(recording.strPlot);
    kodiRecording.SetRecordingId(recording.strRecordingId);
    kodiRecording.SetTitle(recording.strTitle);
    kodiRecording.SetEpisodeName(recording.strEpisodeName);
    kodiRecording.SetDirectory(recording.strDirectory);

    /* TODO: PVR API 5.0.0: Implement this */
    kodiRecording.SetChannelUid(PVR_CHANNEL_INVALID_UID);

    results.Add(kodiRecording);
  }

  return PVR_ERROR_NO_ERROR;
}

PVR_ERROR CPVRDemo::GetRecordingStreamProperties(
    const kodi::addon::PVRRecording& recording,
    std::vector<kodi::addon::PVRStreamProperty>& properties)
{
  properties.emplace_back(PVR_STREAM_PROPERTY_STREAMURL, GetRecordingURL(recording));
  return PVR_ERROR_NO_ERROR;
}

PVR_ERROR CPVRDemo::GetTimerTypes(std::vector<kodi::addon::PVRTimerType>& types)
{
  /* TODO: Implement this to get support for the timer features introduced with PVR API 1.9.7 */
  return PVR_ERROR_NOT_IMPLEMENTED;
}

PVR_ERROR CPVRDemo::GetTimersAmount(int& amount)
{
  amount = m_timers.size();
  return PVR_ERROR_NO_ERROR;
}

PVR_ERROR CPVRDemo::GetTimers(kodi::addon::PVRTimersResultSet& results)
{
  unsigned int i = PVR_TIMER_NO_CLIENT_INDEX + 1;
  for (const auto& timer : m_timers)
  {
    kodi::addon::PVRTimer kodiTimer;

    /* TODO: Implement own timer types to get support for the timer features introduced with PVR API 1.9.7 */
    kodiTimer.SetTimerType(PVR_TIMER_TYPE_NONE);
    kodiTimer.SetClientIndex(i++);
    kodiTimer.SetClientChannelUid(timer.iChannelId);
    kodiTimer.SetStartTime(timer.startTime);
    kodiTimer.SetEndTime(timer.endTime);
    kodiTimer.SetState(timer.state);
    kodiTimer.SetTitle(timer.strTitle);
    kodiTimer.SetSummary(timer.strSummary);

    results.Add(kodiTimer);
  }

  return PVR_ERROR_NO_ERROR;
}

PVR_ERROR CPVRDemo::CallEPGMenuHook(const kodi::addon::PVRMenuhook& menuhook,
                                    const kodi::addon::PVREPGTag& item)
{
  return CallMenuHook(menuhook);
}

PVR_ERROR CPVRDemo::CallChannelMenuHook(const kodi::addon::PVRMenuhook& menuhook,
                                        const kodi::addon::PVRChannel& item)
{
  return CallMenuHook(menuhook);
}

PVR_ERROR CPVRDemo::CallTimerMenuHook(const kodi::addon::PVRMenuhook& menuhook,
                                      const kodi::addon::PVRTimer& item)
{
  return CallMenuHook(menuhook);
}

PVR_ERROR CPVRDemo::CallRecordingMenuHook(const kodi::addon::PVRMenuhook& menuhook,
                                          const kodi::addon::PVRRecording& item)
{
  return CallMenuHook(menuhook);
}

PVR_ERROR CPVRDemo::CallSettingsMenuHook(const kodi::addon::PVRMenuhook& menuhook)
{
  return CallMenuHook(menuhook);
}

PVR_ERROR CPVRDemo::CallMenuHook(const kodi::addon::PVRMenuhook& menuhook)
{
  int iMsg;
  switch (menuhook.GetHookId())
  {
    case 1:
      iMsg = 30010;
      break;
    case 2:
      iMsg = 30011;
      break;
    case 3:
      iMsg = 30012;
      break;
    default:
      return PVR_ERROR_INVALID_PARAMETERS;
  }
  kodi::QueueNotification(QUEUE_INFO, "", kodi::GetLocalizedString(iMsg));

  return PVR_ERROR_NO_ERROR;
}

bool CPVRDemo::LoadDemoData(void)
{
  std::string jsonReaderError;
  Json::CharReaderBuilder jsonReaderBuilder;
  std::unique_ptr<Json::CharReader> const reader(jsonReaderBuilder.newCharReader());

  std::string strSettingsFile = kodi::GetAddonPath("PVRDemoAddonSettings.json");

  std::string fileContents;
  kodi::vfs::CFile settingsFileHandle;

  if (settingsFileHandle.OpenFile(strSettingsFile, ADDON_READ_NO_CACHE))
  {
    char buffer[1024];
    int bytesRead = 0;
    while ((bytesRead = settingsFileHandle.Read(buffer, sizeof(buffer) - 1)) > 0)
      fileContents.append(buffer, bytesRead);

    settingsFileHandle.Close();
  }
  else
  {
    kodi::Log(ADDON_LOG_ERROR, "%s Could not open source file to read: %s", __func__, strSettingsFile.c_str());
  }

  Json::Value jsonValue;
  if (!reader->parse(fileContents.c_str(), fileContents.c_str() + fileContents.length(),
                     &jsonValue, &jsonReaderError))
  {
    kodi::Log(ADDON_LOG_ERROR, "invalid demo data (no/invalid data file found at '%s')",
              strSettingsFile.c_str());
    return false;
  }

  if (jsonValue["demo"].isNull())
  {
    kodi::Log(ADDON_LOG_ERROR, "invalid demo data (no demo entry found)");
    return false;
  }

  /* load channels */
  int iUniqueChannelId = 0;
  Json::Value channels = jsonValue["demo"]["channels"];
  if (channels.isArray())
  {
    for (const Json::Value& c : channels)
    {
      PVRDemoChannel channel;
      if (ScanJSONChannelData(c, ++iUniqueChannelId, channel))
        m_channels.emplace_back(channel);
    }
  }

  /* load channel groups */
  int iUniqueGroupId = 0;
  Json::Value groups = jsonValue["demo"]["channelgroups"];
  if (groups.isArray())
  {
    for (const Json::Value& g : groups)
    {
      PVRDemoChannelGroup group;
      if (ScanJSONChannelGroupData(g, ++iUniqueGroupId, group))
        m_groups.emplace_back(group);
    }
  }

  /* load EPG entries */
  Json::Value entries = jsonValue["demo"]["epg"]["entries"];
  if (entries.isArray())
  {
    for (const Json::Value& e : entries)
    {
      ScanJSONEpgData(e);
    }
  }

  /* load recordings */
  iUniqueGroupId = 0; // reset unique ids
  Json::Value recordings = jsonValue["demo"]["recordings"];
  if (recordings.isArray())
  {
    for (const Json::Value& r : recordings)
    {
      PVRDemoRecording recording;
      if (ScanJSONRecordingData(r, ++iUniqueGroupId, recording))
        m_recordings.emplace_back(recording);
    }
  }

  /* load deleted recordings */
  Json::Value recordingsdeleted = jsonValue["demo"]["recordingsdeleted"];
  if (recordingsdeleted.isArray())
  {
    for (const Json::Value& r : recordingsdeleted)
    {
      PVRDemoRecording recording;
      if (ScanJSONRecordingData(r, ++iUniqueGroupId, recording))
        m_recordingsDeleted.emplace_back(recording);
    }
  }

  /* load timers */
  Json::Value timers = jsonValue["demo"]["timers"];
  if (timers.isArray())
  {
    for (const Json::Value& t : timers)
    {
      PVRDemoTimer timer;
      if (ScanJSONTimerData(t, timer))
        m_timers.emplace_back(timer);
    }
  }

  return true;
}

bool CPVRDemo::GetChannel(const kodi::addon::PVRChannel& channel, PVRDemoChannel& myChannel)
{
  for (const auto& thisChannel : m_channels)
  {
    if (thisChannel.iUniqueId == (int)channel.GetUniqueId())
    {
      myChannel.iUniqueId = thisChannel.iUniqueId;
      myChannel.bRadio = thisChannel.bRadio;
      myChannel.iChannelNumber = thisChannel.iChannelNumber;
      myChannel.iSubChannelNumber = thisChannel.iSubChannelNumber;
      myChannel.iEncryptionSystem = thisChannel.iEncryptionSystem;
      myChannel.strChannelName = thisChannel.strChannelName;
      myChannel.strIconPath = thisChannel.strIconPath;
      myChannel.strStreamURL = thisChannel.strStreamURL;

      return true;
    }
  }

  return false;
}

std::string CPVRDemo::GetRecordingURL(const kodi::addon::PVRRecording& recording)
{
  for (const auto& thisRecording : m_recordings)
  {
    if (thisRecording.strRecordingId == recording.GetRecordingId())
    {
      return thisRecording.strStreamURL;
    }
  }

  return "";
}

bool CPVRDemo::ScanJSONChannelData(const Json::Value& node,
                                   int iUniqueChannelId,
                                   PVRDemoChannel& channel)
{
  std::string strTmp;
  channel.iUniqueId = iUniqueChannelId;

  /* channel name */
  if (!node["name"].isString())
    return false;
  channel.strChannelName = node["name"].asString();

  channel.bRadio = node["radio"].asBool();

  /* channel number */
  if (!node["number"].isInt())
    channel.iChannelNumber = iUniqueChannelId;
  else
    channel.iChannelNumber = node["number"].asInt();

  /* sub channel number */
  channel.iSubChannelNumber = node["subnumber"].asInt();

  /* CAID */
  channel.iEncryptionSystem = node["encryption"].asInt();

  /* icon path */
  if (!node["icon"].isString())
    channel.strIconPath = m_strDefaultIcon;
  else
    channel.strIconPath = ClientPath() + node["icon"].asString();

  /* stream url */
  if (!node["stream"].isString())
    channel.strStreamURL = m_strDefaultMovie;
  else
    channel.strStreamURL = node["stream"].asString();

  channel.bArchive = node["archive"].asBool();

  return true;
}

bool CPVRDemo::ScanJSONChannelGroupData(const Json::Value& node,
                                        int iUniqueGroupId,
                                        PVRDemoChannelGroup& group)
{
  group.iGroupId = iUniqueGroupId;

  /* group name */
  if (!node["name"].isString())
    return false;
  group.strGroupName = node["name"].asString();

  /* radio/TV */
  group.bRadio = node["radio"].asBool();

  /* sort position */
  group.iPosition = node["position"].asInt();

  /* members */
  if (node["members"].isArray()) {
    for (const Json::Value& m : node["members"])
    {
      int iChannelId = m.asInt();
      if (iChannelId > -1)
        group.members.emplace_back(iChannelId);
    }
  }

  return true;
}

bool CPVRDemo::ScanJSONEpgData(const Json::Value& node)
{
  PVRDemoEpgEntry entry;

  /* broadcast id */
  if (!node["broadcastid"].isInt())
    return false;
  entry.iBroadcastId = node["broadcastid"].asInt();

  /* channel id */
  if (!node["channelid"].isInt())
    return false;
  PVRDemoChannel& channel = m_channels.at(node["channelid"].asInt() - 1);
  entry.iChannelId = channel.iUniqueId;

  /* title */
  if (!node["title"].isString())
    return false;
  entry.strTitle = node["title"].asString();

  /* start */
  if (!node["start"].isInt())
    return false;
  entry.startTime = node["start"].asInt();

  /* end */
  if (!node["end"].isInt())
    return false;
  entry.endTime = node["end"].asInt();

  /* plot */
  if (node["plot"].isString())
    entry.strPlot = node["plot"].asString();

  /* plot outline */
  if (node["plotoutline"].isString())
    entry.strPlotOutline = node["plot"].asString();

  if (!node["series"].isInt())
    entry.iSeriesNumber = EPG_TAG_INVALID_SERIES_EPISODE;
  else
    entry.iSeriesNumber = node["series"].asInt();

  if (!node["episode"].isInt())
    entry.iEpisodeNumber = EPG_TAG_INVALID_SERIES_EPISODE;
  else
    entry.iEpisodeNumber = node["episode"].asInt();

  if (node["episodetitle"].isString())
    entry.strEpisodeName = node["episodetitle"].asString();

  /* icon path */
  if (node["icon"].isString())
    entry.strIconPath = node["icon"].asString();

  /* genre type */
  entry.iGenreType = node["genretype"].asInt();

  /* genre subtype */
  entry.iGenreSubType = node["genresubtype"].asInt();

  kodi::Log(ADDON_LOG_DEBUG, "loaded EPG entry '%s' channel '%d' start '%d' end '%d'",
            entry.strTitle.c_str(), entry.iChannelId, entry.startTime, entry.endTime);

  channel.epg.emplace_back(entry);

  return true;
}

bool CPVRDemo::ScanJSONRecordingData(const Json::Value& node,
                                     int iUniqueGroupId,
                                     PVRDemoRecording& recording)
{
  recording.strRecordingId = std::to_string(iUniqueGroupId);

  /* radio/TV */
  recording.bRadio = node["radio"].asBool();

  /* recording title */
  if (!node["title"].isString())
    return false;
  recording.strTitle = node["title"].asString();

  /* recording url */
  if (!node["url"].isString())
    recording.strStreamURL = m_strDefaultMovie;
  else
    recording.strStreamURL = node["url"].asString();

  /* recording path */
  if (node["directory"].isString())
    recording.strDirectory = node["directory"].asString();

  /* channel name */
  if (node["channelname"].isString())
    recording.strChannelName = node["channelname"].asString();

  /* plot */
  if (node["plot"].isString())
    recording.strPlot = node["plot"].asString();

  /* plot outline */
  if (node["plotoutline"].isString())
    recording.strPlotOutline = node["plot"].asString();

  /* Episode Name */
  if (node["episodetitle"].isString())
    recording.strEpisodeName = node["episodetitle"].asString();

  /* Series Number */
  if (!node["series"].isInt())
    recording.iSeriesNumber = PVR_RECORDING_INVALID_SERIES_EPISODE;
  else
    recording.iSeriesNumber = node["series"].asInt();

  /* Episode Number */
  if (!node["episode"].isInt())
    recording.iEpisodeNumber = PVR_RECORDING_INVALID_SERIES_EPISODE;
  else
    recording.iEpisodeNumber = node["episode"].asInt();

  /* genre type */
  recording.iGenreType = node["genretype"].asInt();

  /* genre subtype */
  recording.iGenreSubType = node["genresubtype"].asInt();

  /* duration */
  recording.iDuration = node["duration"].asInt();

  /* recording time */
  if (node["time"].isString())
  {
    time_t timeNow = time(nullptr);
    struct tm* now = localtime(&timeNow);

    auto delim = node["time"].asString().find(':');
    if (delim != std::string::npos)
    {
      sscanf(node["time"].asString().c_str(), "%d:%d", &now->tm_hour, &now->tm_min);
      now->tm_mday--; // yesterday

      recording.recordingTime = mktime(now);
    }
  }

  return true;
}

bool CPVRDemo::ScanJSONTimerData(const Json::Value& node, PVRDemoTimer& timer)
{
  time_t timeNow = time(nullptr);
  struct tm* now = localtime(&timeNow);

  /* channel id */
  if (!node["channelid"].isInt())
    return false;
  PVRDemoChannel& channel = m_channels.at(node["channelid"].asInt() - 1);
  timer.iChannelId = channel.iUniqueId;

  /* state */
  if (node["state"].isInt())
    timer.state = (PVR_TIMER_STATE)node["state"].asInt();

  /* title */
  if (!node["title"].isString())
    return false;
  timer.strTitle = node["title"].asString();

  /* summary */
  if (!node["summary"].isString())
    return false;
  timer.strSummary = node["summary"].asString();

  /* start time */
  if (node["starttime"].isString())
  {
    auto delim = node["starttime"].asString().find(':');
    if (delim != std::string::npos)
    {
      sscanf(node["starttime"].asString().c_str(), "%d:%d", &now->tm_hour, &now->tm_min);
      timer.startTime = mktime(now);
    }
  }

  /* end time */
  if (node["endtime"].isString())
  {
    auto delim = node["endtime"].asString().find(':');
    if (delim != std::string::npos)
    {
      sscanf(node["endtime"].asString().c_str(), "%d:%d", &now->tm_hour, &now->tm_min);
      timer.endTime = mktime(now);
    }
  }

  kodi::Log(ADDON_LOG_DEBUG, "loaded timer '%s' channel '%d' start '%d' end '%d'",
            timer.strTitle.c_str(), timer.iChannelId, timer.startTime, timer.endTime);
  return true;
}

ADDONCREATOR(CPVRDemo)
