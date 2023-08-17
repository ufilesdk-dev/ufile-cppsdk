#ifndef _UFILESDK_CPP_DEMO_MAIN_
#define _UFILESDK_CPP_DEMO_MAIN_

#include <iostream>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <ufile-cppsdk/api.h>
#include <unistd.h>

void help() {

  std::cerr << "./demo put bucket key file" << std::endl;
  std::cerr << "./demo putstr bucket key str" << std::endl;
  std::cerr << "./demo mput bucket key file" << std::endl;
  std::cerr << "./demo delete bucket key" << std::endl;
  std::cerr << "./demo download bucket key storedfile [range_begin-range_end]"
            << std::endl;
  std::cerr << "./demo download2 bucket key storedfile [range_begin-range_end]"
            << std::endl;
  std::cerr << "./demo showurl bucket key [expires]" << std::endl;
  std::cerr << "./demo mputcopypart src_bucket src_key src_offset dst_bucket "
               "dst_key size [mimetype]"
            << std::endl;
  std::cerr << "./demo puttagging bucket key tag1 value1 [tag2 value2 ...]"
            << std::endl;
  std::cerr << "./demo gettagging bucket key" << std::endl;
  std::cerr << "./demo deletetagging bucket key" << std::endl;
}

//普通上传
int putfile(int argc, char **argv) {

  if (argc != 3) {
    std::cerr << "./demo put bucket key file" << std::endl;
    return -1;
  }

  ucloud::cppsdk::api::UFilePut uploader;
  int ret = uploader.PutFile(argv[0], argv[1], argv[2]);
  if (ret) {
    std::cerr << "putfile error: retcode=" << UFILE_LAST_RETCODE()
              << ", errmsg=" << UFILE_LAST_ERRMSG() << std::endl;
    return ret;
  }
  std::cout << "put file success" << std::endl;
  return 0;
}

int putstr(int argc, char **argv) {

  if (argc != 3) {
    std::cerr << "./demo putstr bucket key str" << std::endl;
    return -1;
  }

  ucloud::cppsdk::api::UFilePut uploader;
  int ret = uploader.Put(argv[0], argv[1], argv[2], strlen(argv[2]));
  if (ret) {
    std::cerr << "putstr error: retcode=" << UFILE_LAST_RETCODE()
              << ", errmsg=" << UFILE_LAST_ERRMSG() << std::endl;
    return ret;
  }
  std::cout << "put str success" << std::endl;
  return 0;
}

//分片上传
int mputfile(int argc, char **argv) {

  if (argc != 3) {
    std::cerr << "./demo mput bucket key file" << std::endl;
    return -1;
  }

  ucloud::cppsdk::api::UFileMput uploader;
  uploader.SetResource(argv[0], argv[1]);
  uploader.SetUploadFile(argv[2]);
  int ret = uploader.MInit();
  if (ret) {
    std::cerr << "minit error: retcode=" << UFILE_LAST_RETCODE()
              << ", errmsg=" << UFILE_LAST_ERRMSG() << std::endl;
    return ret;
  }

  struct stat st;
  if (stat(argv[2], &st)) {
    std::cerr << strerror(errno) << std::endl;
    return -1;
  }

  ssize_t blk = 0;
  while (uploader.UploadedSize() < size_t(st.st_size)) {
    for (int i = 0; i < 20; ++i)
      std::cerr << '\b';
    std::cerr << "finished: " << uploader.UploadedSize() * 100 / st.st_size
              << "%";
    ret = uploader.MUpload(blk++);
    if (ret) {
      std::cerr << "mupload error: retcode=" << UFILE_LAST_RETCODE()
                << ", errmsg=" << UFILE_LAST_ERRMSG() << std::endl;
      return ret;
    }
  }
  std::cerr << std::endl;
  ret = uploader.MFinish(NULL);
  if (ret) {
    std::cerr << "mupload error: retcode=" << UFILE_LAST_RETCODE()
              << ", errmsg=" << UFILE_LAST_ERRMSG() << std::endl;
    return ret;
  }
  std::cout << "mput file success" << std::endl;
  return 0;
}

// 分片拷贝
int mputcopypart(int argc, char **argv) {

  if (argc < 6 || argc > 7) {
    std::cerr << "./demo mputcopypart src_bucket src_key src_offset dst_bucket "
                 "dst_key size [mimetype]"
              << std::endl;
    return -1;
  }

  std::string mimetype = "application/octet-stream";
  if (argc == 7) {
    mimetype = argv[6];
  }

  ucloud::cppsdk::api::UFileMput uploader;
  uploader.SetResource(argv[3], argv[4]);
  int ret = uploader.MInit();
  if (ret) {
    std::cerr << "minit error: retcode=" << UFILE_LAST_RETCODE()
              << ", errmsg=" << UFILE_LAST_ERRMSG() << std::endl;
    return ret;
  }

  ssize_t blk = 0;
  size_t src_offset = std::stoull(argv[2]);
  size_t total_size = std::stoull(argv[5]);
  while (uploader.UploadedSize() < total_size) {
    for (int i = 0; i < 20; ++i)
      std::cerr << '\b';
    std::cerr << "finished: " << uploader.UploadedSize() * 100 / total_size
              << "%\n";
    size_t left = total_size - uploader.UploadedSize();
    size_t blk_size = uploader.BlkSize();
    if (left < blk_size) {
      blk_size = left;
    }
    ret = uploader.MUploadCopyPart(blk++, argv[0], argv[1], src_offset,
                                   blk_size, mimetype);
    if (ret) {
      std::cerr << "mputcopypart error: retcode=" << UFILE_LAST_RETCODE()
                << ", errmsg=" << UFILE_LAST_ERRMSG() << std::endl;
      return ret;
    }
    src_offset += blk_size;
  }
  std::cerr << std::endl;
  ret = uploader.MFinish(NULL);
  if (ret) {
    std::cerr << "mputcopypart error: retcode=" << UFILE_LAST_RETCODE()
              << ", errmsg=" << UFILE_LAST_ERRMSG() << std::endl;
    return ret;
  }
  std::cout << "mputcopypart success" << std::endl;
  return 0;
}

//下载到文件
int download_as_file(int argc, char **argv) {

  if (argc != 3 && argc != 4) {
    std::cerr << "./demo download bucket key storedfile [range_bagin-range_end]"
              << std::endl;
    return -1;
  }

  int ret = 0;
  ucloud::cppsdk::api::UFileDownload downloader;
  std::pair<ssize_t, ssize_t> range;
  ssize_t begin = -1, end = -1;
  if (argc == 4) {
    sscanf(argv[3], "%zd-%zd", &begin, &end);
    range = std::make_pair(begin, end);
    ret = downloader.DownloadAsFile(argv[0], argv[1], argv[2], &range);
  } else {
    ret = downloader.DownloadAsFile(argv[0], argv[1], argv[2]);
  }

  if (ret) {
    std::cerr << "download error: retcode=" << UFILE_LAST_RETCODE()
              << ", errmsg=" << UFILE_LAST_ERRMSG() << std::endl;
    return ret;
  }
  std::cout << "download file success" << std::endl;
  return 0;
}

//下载到一个流中
int download_as_stream(int argc, char **argv) {

  if (argc != 3 && argc != 4) {
    std::cerr
        << "./demo download2 bucket key storedfile [range_begin-range_end]"
        << std::endl;
    return -1;
  }

  std::ofstream ofs(argv[2], std::ofstream::out | std::ofstream::app);
  if (!ofs) {
    std::cerr << "open " << argv[2] << " failed" << std::endl;
    return -1;
  }

  int ret = 0;
  ucloud::cppsdk::api::UFileDownload downloader;
  std::pair<ssize_t, ssize_t> range;
  ssize_t begin = -1, end = -1;
  if (argc == 4) {
    sscanf(argv[3], "%zd-%zd", &begin, &end);
    range = std::make_pair(begin, end);
    ret = downloader.Download(argv[0], argv[1], &ofs, &range);
  } else {
    ret = downloader.Download(argv[0], argv[1], &ofs);
  }

  if (ret) {
    std::cerr << "download error: retcode=" << UFILE_LAST_RETCODE()
              << ", errmsg=" << UFILE_LAST_ERRMSG() << std::endl;
    ofs.close();
    return ret;
  }
  std::cout << "download file success" << std::endl;
  ofs.close();
  return 0;
}

//设置tagging
int put_tagging(int argc, char **argv) {
  if (argc < 4 || argc % 2 != 0) {
    std::cerr << "./demo puttagging bucket key tag1 value1 [tag2 value2 ...]"
              << std::endl;
    return -1;
  }

  std::string bucket = argv[0];
  std::string key = argv[1];

  argv += 2;
  argc -= 2;
  std::map<std::string, std::string> tags;
  while (argc > 0) {
    std::string tag = argv[0];
    std::string value = argv[1];
    auto insert_ret = tags.insert(std::make_pair(tag, value));
    if (!insert_ret.second) {
      std::cerr << "duplicate tag: " << tag << std::endl;
      return -1;
    }

    argv += 2;
    argc -= 2;
  }
  ucloud::cppsdk::api::UFileTagging putter;
  int ret = putter.PutTagging(bucket, key, tags);
  if (ret) {
    std::cerr << "put tagging error: retcode=" << UFILE_LAST_RETCODE()
              << ", errmsg=" << UFILE_LAST_ERRMSG() << std::endl;
    return ret;
  }
  std::cout << "put tagging success" << std::endl;
  return 0;
}

//获取tagging
int get_tagging(int argc, char **argv) {
  if (argc != 2) {
    std::cerr << "./demo gettagging bucket key" << std::endl;
    return -1;
  }

  std::string bucket = argv[0];
  std::string key = argv[1];

  std::map<std::string, std::string> tags;
  ucloud::cppsdk::api::UFileTagging getter;
  int ret = getter.GetTagging(bucket, key, &tags);
  if (ret) {
    std::cerr << "get tagging error: retcode=" << UFILE_LAST_RETCODE()
              << ", errmsg=" << UFILE_LAST_ERRMSG() << std::endl;
    return ret;
  }
  std::cout << "get tagging success" << std::endl;
  std::cout << "{" << std::endl;
  for (auto &tag : tags) {
    std::cout << "  " << tag.first << ": " << tag.second << std::endl;
  }
  std::cout << "}" << std::endl;
  return 0;
}

//删除tagging
int delete_tagging(int argc, char **argv) {
  if (argc != 2) {
    std::cerr << "./demo deletetagging bucket key" << std::endl;
    return -1;
  }

  std::string bucket = argv[0];
  std::string key = argv[1];

  ucloud::cppsdk::api::UFileTagging deleter;
  int ret = deleter.DeleteTagging(bucket, key);
  if (ret) {
    std::cerr << "delete tagging error: retcode=" << UFILE_LAST_RETCODE()
              << ", errmsg=" << UFILE_LAST_ERRMSG() << std::endl;
    return ret;
  }
  std::cout << "delete tagging success" << std::endl;
  return 0;
}

int showurl(int argc, char **argv) {

  size_t expires = 0;
  if (argc != 3 && argc != 2) {
    std::cerr << "./demo showurl bucket key [expires]" << std::endl;
    return -1;
  } else {
    if (argc == 3)
      expires = strtoul(argv[2], NULL, 10);
  }

  ucloud::cppsdk::api::UFileDownload downloader;
  std::cerr << downloader.DownloadURL(argv[0], argv[1], expires) << std::endl;
  return 0;
}

int deletefile(int argc, char **argv) {

  if (argc != 2) {
    std::cerr << "./demo delete bucket key" << std::endl;
    return -1;
  }

  ucloud::cppsdk::api::UFileDelete deleter;
  int ret = deleter.Delete(argv[0], argv[1]);
  if (ret) {
    std::cerr << "delete error: retcode=" << UFILE_LAST_RETCODE()
              << ", errmsg=" << UFILE_LAST_ERRMSG() << std::endl;
    return ret;
  }
  std::cout << "delete file success" << std::endl;
  return 0;
}

int dispatch(int argc, char **argv) {

  int ret = 0;
  char *cmd = argv[0];
  if (memcmp(cmd, "put", strlen(cmd)) == 0) {
    ret = putfile(argc - 1, argv + 1);
  } else if (memcmp(cmd, "mput", strlen(cmd)) == 0) {
    ret = mputfile(argc - 1, argv + 1);
  } else if (memcmp(cmd, "download", strlen(cmd)) == 0) {
    ret = download_as_file(argc - 1, argv + 1);
  } else if (memcmp(cmd, "download2", strlen(cmd)) == 0) {
    ret = download_as_stream(argc - 1, argv + 1);
  } else if (memcmp(cmd, "showurl", strlen(cmd)) == 0) {
    ret = showurl(argc - 1, argv + 1);
  } else if (memcmp(cmd, "delete", strlen(cmd)) == 0) {
    ret = deletefile(argc - 1, argv + 1);
  } else if (memcmp(cmd, "putstr", strlen(cmd)) == 0) {
    ret = putstr(argc - 1, argv + 1);
  } else if (memcmp(cmd, "mputcopypart", strlen(cmd)) == 0) {
    ret = mputcopypart(argc - 1, argv + 1);
  } else if (memcmp(cmd, "puttagging", strlen(cmd)) == 0) {
    ret = put_tagging(argc - 1, argv + 1);
  } else if (memcmp(cmd, "gettagging", strlen(cmd)) == 0) {
    ret = get_tagging(argc - 1, argv + 1);
  } else if (memcmp(cmd, "deletetagging", strlen(cmd)) == 0) {
    ret = delete_tagging(argc - 1, argv + 1);
  } else {
    std::cerr << "unknown command: " << cmd << std::endl;
    help();
    ret = -1;
  }
  return ret;
}

int main(int argc, char **argv) {

  if (argc < 2 || memcmp(argv[1], "-h", 2) == 0 ||
      memcmp(argv[1], "--help", 6) == 0) {
    help();
    exit(0);
  }

  int ret = dispatch(argc - 1, argv + 1);
  exit(ret);
}

#endif
