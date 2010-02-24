"""Tag the sandbox for release, make source and doc tarballs.

Requires Python 2.6

Example of invocation (use to test the script):
python makerelease.py --force --retag --platform=msvc6,msvc71,msvc80,mingw 0.5.0 0.6.0-dev

Example of invocation when doing a release:
python makerelease.py 0.5.0 0.6.0-dev
"""
import os.path
import subprocess
import sys
import doxybuild
import subprocess
import xml.etree.ElementTree as ElementTree
import shutil
import urllib2
from devtools import antglob, fixeol, tarball

SVN_ROOT = 'https://jsoncpp.svn.sourceforge.net/svnroot/jsoncpp/'
SVN_TAG_ROOT = SVN_ROOT + 'tags/jsoncpp'
SCONS_LOCAL_URL = 'http://sourceforge.net/projects/scons/files/scons-local/1.2.0/scons-local-1.2.0.tar.gz/download'

def set_version( version ):
    with open('version','wb') as f:
        f.write( version.strip() )

def rmdir_if_exist( dir_path ):
    if os.path.isdir( dir_path ):
        shutil.rmtree( dir_path )

class SVNError(Exception):
    pass

def svn_command( command, *args ):
    cmd = ['svn', '--non-interactive', command] + list(args)
    print 'Running:', ' '.join( cmd )
    process = subprocess.Popen( cmd,
                                stdout=subprocess.PIPE,
                                stderr=subprocess.STDOUT )
    stdout = process.communicate()[0]
    if process.returncode:
        error = SVNError( 'SVN command failed:\n' + stdout )
        error.returncode = process.returncode
        raise error
    return stdout

def check_no_pending_commit():
    """Checks that there is no pending commit in the sandbox."""
    stdout = svn_command( 'status', '--xml' )
    etree = ElementTree.fromstring( stdout )
    msg = []
    for entry in etree.getiterator( 'entry' ):
        path = entry.get('path')
        status = entry.find('wc-status').get('item')
        if status != 'unversioned':
            msg.append( 'File "%s" has pending change (status="%s")' % (path, status) )
    if msg:
        msg.insert(0, 'Pending change to commit found in sandbox. Commit them first!' )
    return '\n'.join( msg )

def svn_join_url( base_url, suffix ):
    if not base_url.endswith('/'):
        base_url += '/'
    if suffix.startswith('/'):
        suffix = suffix[1:]
    return base_url + suffix

def svn_check_if_tag_exist( tag_url ):
    """Checks if a tag exist.
    Returns: True if the tag exist, False otherwise.
    """
    try:
        list_stdout = svn_command( 'list', tag_url )
    except SVNError, e:
        if e.returncode != 1 or not str(e).find('tag_url'):
            raise e
        # otherwise ignore error, meaning tag does not exist
        return False
    return True

def svn_tag_sandbox( tag_url, message ):
    """Makes a tag based on the sandbox revisions.
    """
    svn_command( 'copy', '-m', message, '.', tag_url )

def svn_remove_tag( tag_url, message ):
    """Removes an existing tag.
    """
    svn_command( 'delete', '-m', message, tag_url )

def svn_export( tag_url, export_dir ):
    """Exports the tag_url revision to export_dir.
       Target directory, including its parent is created if it does not exist.
       If the directory export_dir exist, it is deleted before export proceed.
    """
    rmdir_if_exist( export_dir )
    svn_command( 'export', tag_url, export_dir )

def fix_sources_eol( dist_dir ):
    """Set file EOL for tarball distribution.
    """
    print 'Preparing exported source file EOL for distribution...'
    prune_dirs = antglob.prune_dirs + 'scons-local* ./build* ./libs ./dist'
    win_sources = antglob.glob( dist_dir, 
        includes = '**/*.sln **/*.vcproj',
        prune_dirs = prune_dirs )
    unix_sources = antglob.glob( dist_dir,
        includes = '''**/*.h **/*.cpp **/*.inl **/*.txt **/*.dox **/*.py **/*.html **/*.in
        sconscript *.json *.expected AUTHORS LICENSE''',
        excludes = antglob.default_excludes + 'scons.py sconsign.py scons-*',
        prune_dirs = prune_dirs )
    for path in win_sources:
        fixeol.fix_source_eol( path, is_dry_run = False, verbose = True, eol = '\r\n' )
    for path in unix_sources:
        fixeol.fix_source_eol( path, is_dry_run = False, verbose = True, eol = '\n' )

def download( url, target_path ):
    """Download file represented by url to target_path.
    """
    f = urllib2.urlopen( url )
    try:
        data = f.read()
    finally:
        f.close()
    fout = open( target_path, 'wb' )
    try:
        fout.write( data )
    finally:
        fout.close()

def check_compile( distcheck_top_dir, platform ):
    cmd = [sys.executable, 'scons.py', 'platform=%s' % platform, 'check']
    print 'Running:', ' '.join( cmd )
    log_path = os.path.join( distcheck_top_dir, 'build-%s.log' % platform )
    flog = open( log_path, 'wb' )
    try:
        process = subprocess.Popen( cmd,
                                    stdout=flog,
                                    stderr=subprocess.STDOUT,
                                    cwd=distcheck_top_dir )
        stdout = process.communicate()[0]
        status = (process.returncode == 0)
    finally:
        flog.close()
    return (status, log_path)


def main():
    usage = """%prog release_version next_dev_version
Update 'version' file to release_version and commit.
Generates the document tarball.
Tags the sandbox revision with release_version.
Update 'version' file to next_dev_version and commit.

Performs an svn export of tag release version, and build a source tarball.    

Must be started in the project top directory.

Warning: --force should only be used when developping/testing the release script.
"""
    from optparse import OptionParser
    parser = OptionParser(usage=usage)
    parser.allow_interspersed_args = False
    parser.add_option('--dot', dest="dot_path", action='store', default=doxybuild.find_program('dot'),
        help="""Path to GraphViz dot tool. Must be full qualified path. [Default: %default]""")
    parser.add_option('--doxygen', dest="doxygen_path", action='store', default=doxybuild.find_program('doxygen'),
        help="""Path to Doxygen tool. [Default: %default]""")
    parser.add_option('--force', dest="ignore_pending_commit", action='store_true', default=False,
        help="""Ignore pending commit. [Default: %default]""")
    parser.add_option('--retag', dest="retag_release", action='store_true', default=False,
        help="""Overwrite release existing tag if it exist. [Default: %default]""")
    parser.add_option('-p', '--platforms', dest="platforms", action='store', default='',
        help="""Comma separated list of platform passed to scons for build check.""")
    parser.add_option('--no-test', dest="no_test", action='store', default=False,
        help="""Skips build check.""")
    parser.enable_interspersed_args()
    options, args = parser.parse_args()

    if len(args) < 1:
        parser.error( 'release_version missing on command-line.' )
    release_version = args[0]

    if not options.platforms and not options.no_test:
        parser.error( 'You must specify either --platform or --no-test option.' )

    if options.ignore_pending_commit:
        msg = ''
    else:
        msg = check_no_pending_commit()
    if not msg:
        print 'Setting version to', release_version
        set_version( release_version )
        tag_url = svn_join_url( SVN_TAG_ROOT, release_version )
        if svn_check_if_tag_exist( tag_url ):
            if options.retag_release:
                svn_remove_tag( tag_url, 'Overwriting previous tag' )
            else:
                print 'Aborting, tag %s already exist. Use --retag to overwrite it!' % tag_url
                sys.exit( 1 )
        svn_tag_sandbox( tag_url, 'Release ' + release_version )

        print 'Generated doxygen document...'
        doxybuild.build_doc( options, make_release=True )
        
        export_dir = 'dist/export'
        svn_export( tag_url, export_dir )
        fix_sources_eol( export_dir )
        
        source_dir = 'jsoncpp-src-' + release_version
        source_tarball_path = 'dist/%s.tar.gz' % source_dir
        print 'Generating source tarball to', source_tarball_path
        tarball.make_tarball( source_tarball_path, [export_dir], export_dir, prefix_dir=source_dir )

        # Decompress source tarball, download and install scons-local
        distcheck_dir = 'dist/distcheck'
        distcheck_top_dir = distcheck_dir + '/' + source_dir
        print 'Decompressing source tarball to', distcheck_dir
        rmdir_if_exist( distcheck_dir )
        tarball.decompress( source_tarball_path, distcheck_dir )
        scons_local_path = 'dist/scons-local.tar.gz'
        print 'Downloading scons-local to', scons_local_path
        download( SCONS_LOCAL_URL, scons_local_path )
        print 'Decompressing scons-local to', distcheck_top_dir
        tarball.decompress( scons_local_path, distcheck_top_dir )

        # Run compilation
        print 'Compiling decompressed tarball'
        all_build_status = True
        for platform in options.platforms.split(','):
            print 'Testing platform:', platform
            build_status, log_path = check_compile( distcheck_top_dir, platform )
            print 'see build log:', log_path
            print build_status and '=> ok' or '=> FAILED'
            all_build_status = all_build_status and build_status
        if not build_status:
            print 'Testing failed on at least one platform, aborting...'
            svn_remove_tag( tag_url, 'Removing tag due to failed testing' )
            sys.exit(1)
        
        #@todo:
        # ?upload documentation
    else:
        sys.stderr.write( msg + '\n' )
 
if __name__ == '__main__':
    main()
