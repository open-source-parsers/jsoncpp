# Copyright 2026 The JsonCpp Authors
# Distributed under MIT license, or public domain if desired and recognized.
# See file LICENSE for details.

import contextlib
import io
import os
import sys
import tempfile
import unittest
from unittest import mock

import runjsontests


class RunnerTest(unittest.TestCase):
    def test_arguments_are_not_split(self):
        with contextlib.redirect_stderr(io.StringIO()):
            status, output = runjsontests.getStatusOutput([
                sys.executable, '-c',
                'import sys; print(repr(sys.argv[1:]))',
                'argument with spaces', 'quote"value', 'single\'quote'])
        self.assertEqual(status, 0)
        self.assertEqual(output.strip(),
                         repr(['argument with spaces', 'quote"value',
                               'single\'quote']))

    def test_exit_status_and_output(self):
        with contextlib.redirect_stderr(io.StringIO()):
            status, output = runjsontests.getStatusOutput([
                sys.executable, '-c',
                'import sys; print("output"); sys.exit(7)'])
        self.assertEqual(status, 7)
        self.assertEqual(output, 'output\n')

    def test_missing_executable_returns_failure(self):
        with tempfile.TemporaryDirectory() as directory:
            missing = os.path.join(directory, 'missing executable')
            with contextlib.redirect_stderr(io.StringIO()):
                status, output = runjsontests.getStatusOutput([missing])
        self.assertTrue(status)
        self.assertEqual(output, '')

    @unittest.skipIf(os.name == 'nt', 'POSIX executable permission test')
    def test_non_executable_file_returns_failure(self):
        with tempfile.TemporaryDirectory() as directory:
            path = os.path.join(directory, 'not executable')
            with open(path, 'w') as executable:
                executable.write('not executable')
            os.chmod(path, 0o600)
            with contextlib.redirect_stderr(io.StringIO()):
                status, output = runjsontests.getStatusOutput([path])
        self.assertTrue(status)
        self.assertEqual(output, '')

    def test_missing_executable_does_not_stop_remaining_fixtures(self):
        with tempfile.TemporaryDirectory() as directory:
            missing = os.path.join(directory, 'missing executable')
            for name in ['first.json', 'second.json']:
                with open(os.path.join(directory, name), 'w') as fixture:
                    fixture.write('{}')
            output = io.StringIO()
            with contextlib.redirect_stdout(output):
                with contextlib.redirect_stderr(io.StringIO()):
                    with self.assertRaises(runjsontests.FailError):
                        runjsontests.runAllTests(missing, directory)
        self.assertIn('first.json', output.getvalue())
        self.assertIn('second.json', output.getvalue())
        self.assertIn('Test results: 0 passed, 2 failed.', output.getvalue())

    def check_command(self, input_path, expected_options, use_valgrind=False):
        executable = os.path.join('build space', 'runner executable')
        with mock.patch.object(runjsontests, 'getStatusOutput',
                               return_value=(1, 'expected parse failure')) as run:
            with contextlib.redirect_stdout(io.StringIO()):
                runjsontests.runAllTests(executable, input_path,
                                         use_valgrind=use_valgrind)
        expected = runjsontests.VALGRIND_CMD.split() if use_valgrind else []
        expected += [executable] + expected_options
        expected += ['--json-writer', 'StyledWriter', input_path]
        run.assert_called_once_with(expected)

    def test_parse_only_keeps_spaced_paths(self):
        self.check_command(os.path.join('fixture space', 'fail_example.json'),
                           ['--parse-only'])

    def test_strict_parse_only_options(self):
        self.check_command(os.path.join('fixture space', 'fail_strict_case.json'),
                           ['--parse-only', '--strict'])

    def test_json_checker_options(self):
        self.check_command(os.path.join('jsonchecker', 'fail1.json'),
                           ['--parse-only', '--strict'])

    def test_valgrind_prefix(self):
        self.check_command(os.path.join('fixture space', 'fail_example.json'),
                           ['--parse-only'], use_valgrind=True)


if __name__ == '__main__':
    unittest.main()
