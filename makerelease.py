"""Tag the sandbox for release, make source and doc tarballs.

Requires Python 2.6

Example of invocation (use to test the script):
python makerelease.py --force --retag 0.5.0 0.6.0-dev

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

SVN_ROOT = 'https://jsoncpp.svn.sourceforge.net/svnroot/jsoncpp/'
SVN_TAG_ROOT = SVN_ROOT + 'tags/jsoncpp'

def set_version( version ):
    with open('version','wb') as f:
        f.write( version.strip() )

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
    if os.path.isdir( export_dir ):
        shutil.rmtree( export_dir )
    svn_command( 'export', tag_url, export_dir )

def main():
    usage = """%prog release_version next_dev_version
Update 'version' file to release_version and commit.
Generates the document tarball.
Tags the sandbox revision with release_version.
Update 'version' file to next_dev_version and commit.

Performs an svn export of tag release version, and build a source tarball.    

Must be started in the project top directory.    
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
    parser.enable_interspersed_args()
    options, args = parser.parse_args()

    if len(args) < 1:
        parser.error( 'release_version missing on command-line.' )
    release_version = args[0]

    if options.ignore_pending_commit:
        msg = ''
    else:
        msg = check_no_pending_commit()
    if not msg:
        print 'Setting version to', release_version
        set_version( release_version )
        tag_url = svn_join_url( SVN_TAG_ROOT, release_version )
##        if svn_check_if_tag_exist( tag_url ):
##            if options.retag_release:
##                svn_remove_tag( tag_url, 'Overwriting previous tag' )
##            else:
##                print 'Aborting, tag %s already exist. Use --retag to overwrite it!' % tag_url
##                sys.exit( 1 )
##        svn_tag_sandbox( tag_url, 'Release ' + release_version )
##        print 'Generated doxygen document...'
##        doxybuild.build_doc( options, make_release=True )
        svn_export( tag_url, 'dist/distcheck' )
        #@todo:
        # fix-eol
        # source tarball
        # decompress source tarball
        # ?compile & run & check
        # ?upload documentation
    else:
        sys.stderr.write( msg + '\n' )
 
if __name__ == '__main__':
    main()
