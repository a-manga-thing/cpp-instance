project('instance', 'cpp', default_options : ['optimization=2'])

drogon = find_program('drogon_ctl')
cc = meson.get_compiler('cpp')
add_project_arguments(['--std=c++2a'], language : 'cpp')
add_project_link_arguments(['--std=c++2a'], language : 'cpp')

cmake = import('cmake')

#ipfs_proj = cmake.subproject('cpp-ipfs-http-client')
#ipfs_lib = ipfs_proj.dependency('ipfs-http-client')

rnp_proj = cmake.subproject('rnp')
rnp_lib = rnp_proj.dependency('librnp')

dep = [
    cc.find_library('z'),
    cc.find_library('dl'),
    cc.find_library('cares'),
    dependency('MagickWand'),
    dependency('sqlite3'),
    dependency('jsoncpp'),
    dependency('openssl'),
    dependency('threads'),
    dependency('Drogon'),
    dependency('uuid'),
    dependency('fmt'),
    dependency('gpgme'),
    rnp_lib,
#    ipfs_lib,
]

inc = [
    include_directories('.'),
    include_directories('models'),
    include_directories('include'),
]

src = [
    'src/Main.cc',
    'src/Util.cc',
    'src/UtilPGP.cc',
    'src/UtilManga.cc',
    'src/Globals.cc',
    'controllers/ChapterSync.cc',
    'controllers/MangaSync.cc',
    'controllers/MangaCtrl.cc',
    'controllers/SyncCtrl.cc',
    'controllers/BaseCtrl.cc',
    'controllers/UserCtrl.cc',
    'models/Artist.cc',
    'models/Chapter.cc',
    'models/MangaTag.cc',
    'models/Tag.cc',
    'models/Author.cc',
    'models/Manga.cc',
    'models/Person.cc',
    'models/Title.cc',
    'models/GlobalState.cc',
    'models/User.cc',
]

executable('instance', sources : src, dependencies : dep, include_directories : inc)
