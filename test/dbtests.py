#!/usr/bin/env python3
import unittest
import signal
import subprocess
import sys
import os

from typing import List


def start_db() -> subprocess.Popen:
    args = ["../simpledb"]
    return subprocess.Popen(args, stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                            stderr=subprocess.PIPE, universal_newlines=True)


def do_command(proc: subprocess.Popen, command: str):
    proc.stdin.write(command + "\n")
    proc.stdin.flush()


def do_sequence(commands: List[str]) -> List[str]:

    proc = start_db()

    try:
        for command in commands:
            do_command(proc, command)

        proc.stdin.close()
        proc.wait(5)

    except subprocess.TimeoutExpired:
        proc.kill()

    except Exception as e:
        print(e)
        print("killing...")
        proc.kill()

    with proc.stdout as outs:
        with proc.stderr as errs:
            return outs.read().splitlines() + errs.read().splitlines()


class TestInsertSelect(unittest.TestCase):

    def setUp(self):
        if os.path.isfile("dbfile"):
            os.remove("dbfile")

    def test_insert_select_row(self):
        expected_result = [
            "db > Executed",
            "db > [1, username, email@email.com]",
            "Executed",
            "db > ",
        ]

        commands = [
            "insert 1 username email@email.com",
            "select",
            ".exit",
        ]

        actual_result = do_sequence(commands)
        self.assertEqual(actual_result, expected_result)

    def test_error_message_on_full_tabale(self):
        expected_result = ["Executed", "db > Error: table full"]

        commands = ["insert {0} user{0} email{0}@email.com".format(
            i) for i in range(1401)]
        commands += [".exit"]

        actual_result = do_sequence(commands)
        self.assertEqual(actual_result[-3], expected_result[0])
        self.assertEqual(actual_result[-2], expected_result[1])

    def test_table_allows_max_length_fields(self):
        username = "a" * 32
        email = "b" * 255

        expected_result = [
            "db > Executed",
            "db > [1, {0}, {1}]".format(username, email),
            "Executed",
            "db > ",
        ]

        commands = [
            "insert 1 {0} {1}".format(username, email),
            "select",
            ".exit",
        ]

        actual_result = do_sequence(commands)
        self.assertEqual(actual_result, expected_result)

    def test_table_rejects_too_long_fields(self):
        username = "a" * 33
        email = "b" * 256

        expected_result = [
            "db > Field is too long",
            "db > Executed",
            "db > ",
        ]

        commands = [
            "insert 1 {0}, {1}".format(username, email),
            "select",
            ".exit",
        ]

        actual_result = do_sequence(commands)
        self.assertEqual(actual_result, expected_result)

    def test_table_rejects_negative_id(self):

        expected_result = [
            "db > Id cannot be negative",
            "db > Executed",
            "db > ",
        ]

        commands = [
            "insert -1 a b",
            "select",
            ".exit",
        ]

        actual_result = do_sequence(commands)
        self.assertEqual(actual_result, expected_result)

    def test_table_persists(self):

        expected_result = [
            "db > Executed",
            "db > ",
        ]

        commands = [
            "insert 1 a b",
            ".exit"
        ]

        actual_result = do_sequence(commands)
        self.assertEqual(actual_result, expected_result)

        expected_result = [
            "db > [1, a, b]",
            "Executed",
            "db > ",
        ]

        commands = [
            "select",
            ".exit",
        ]

        actual_result = do_sequence(commands)
        self.assertEqual(actual_result, expected_result)

    def test_constants_are_constant(self):
        expected_result = [
            "db > Constants: ",
            "Row Size: 293",
            "Common Node Header size: 6",
            "Leaf Node Header Size: 10",
            "Leaf Node Cell Size: 297",
            "Leaf Node Space For Cells: 4086",
            "Leaf Node Max Cell: 13",
            "db > "
        ]

        commands = [
            ".constants",
            ".exit"
        ]

        actual_result = do_sequence(commands)
        self.assertEqual(actual_result, expected_result)

    def test_print_btree(self):
        expected_result = [
            "db > Executed",
            "db > Executed",
            "db > Executed",
            "db > Tree:",
            "  Leaf size: 3",
            "    0 : 1",
            "    1 : 2",
            "    2 : 3",
            "db > "
        ]

        commands = [
            *[f"insert {x} user{x} user{x}@email.com" for x in [3, 1, 2]],
            ".btree",
            ".exit"
        ]

        actual_result = do_sequence(commands)
        self.assertEqual(actual_result, expected_result)

    def test_error_message_on_duplicate_key(self):
        expected_result = [
            "db > Executed",
            "db > Error: duplicate key",
            "db > "
        ]

        commands = [
            "insert 1 user1 user1@email.com",
            "insert 1 user1 user1@email.com",
            ".exit"
        ]

        actual_result = do_sequence(commands)
        self.assertEqual(actual_result, expected_result)


if __name__ == "__main__":
    unittest.main()
