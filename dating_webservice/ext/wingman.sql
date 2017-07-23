-- phpMyAdmin SQL Dump
-- version 4.6.4
-- https://www.phpmyadmin.net/
--
-- Host: 127.0.0.1
-- Generation Time: Feb 05, 2017 at 05:07 AM
-- Server version: 5.7.14
-- PHP Version: 7.0.10

SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
SET time_zone = "+00:00";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;

--
-- Database: `wingman`
--

-- --------------------------------------------------------

--
-- Table structure for table `inboxes`
--

CREATE TABLE `inboxes` (
  `id` int(10) UNSIGNED NOT NULL,
  `userOneId` int(11) NOT NULL,
  `userTwoId` int(11) NOT NULL,
  `timeCreated` datetime NOT NULL,
  `stage` int(11) NOT NULL,
  `userStarterId` int(11) NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_bin;

--
-- Dumping data for table `inboxes`
--

INSERT INTO `inboxes` (`id`, `userOneId`, `userTwoId`, `timeCreated`, `stage`, `userStarterId`) VALUES
(1, 1, 10, '2017-02-04 17:56:40', 0, 1);

-- --------------------------------------------------------

--
-- Table structure for table `inboxmessages`
--

CREATE TABLE `inboxmessages` (
  `id` int(11) NOT NULL,
  `inboxId` int(11) NOT NULL,
  `content` text COLLATE utf8_bin NOT NULL,
  `timeCreated` datetime NOT NULL,
  `fromUserId` int(11) NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_bin;

-- --------------------------------------------------------

--
-- Table structure for table `usercheckins`
--

CREATE TABLE `usercheckins` (
  `id` int(10) UNSIGNED NOT NULL,
  `timeCreated` datetime NOT NULL,
  `userId` int(11) NOT NULL,
  `milesFromCentralPoint` double NOT NULL,
  `latitude` double NOT NULL,
  `longitude` double NOT NULL,
  `city` text COLLATE utf8_bin NOT NULL,
  `state` text COLLATE utf8_bin NOT NULL,
  `postalCode` text COLLATE utf8_bin NOT NULL,
  `address` text COLLATE utf8_bin NOT NULL,
  `country` text COLLATE utf8_bin NOT NULL,
  `street` text COLLATE utf8_bin NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_bin;

--
-- Dumping data for table `usercheckins`
--

INSERT INTO `usercheckins` (`id`, `timeCreated`, `userId`, `milesFromCentralPoint`, `latitude`, `longitude`, `city`, `state`, `postalCode`, `address`, `country`, `street`) VALUES
(20, '2017-02-04 20:32:58', 65, 761.70221310252, 43.771094, -115.664063, '', '', '83716', '', '', ''),
(21, '2017-02-04 20:39:12', 2, 761.70221310252, 43.771094, -115.664063, '', '', '83716', '', '', ''),
(19, '2017-02-04 14:00:18', 101, 1181.3532288447, 47.6764983, -122.3439983, '', '', '98115', '', '', ''),
(18, '2017-02-04 13:55:28', 101, 1181.3532288447, 47.6764983, -122.3439983, '', '', '98115', '', '', '');

-- --------------------------------------------------------

--
-- Table structure for table `userphotos`
--

CREATE TABLE `userphotos` (
  `id` int(11) NOT NULL,
  `path` text COLLATE utf8_bin NOT NULL,
  `isMain` int(11) NOT NULL,
  `userId` int(11) NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_bin;

--
-- Dumping data for table `userphotos`
--

INSERT INTO `userphotos` (`id`, `path`, `isMain`, `userId`) VALUES
(5, 'C:\\wamp64\\www\\wingman\\uploads\\img_2_0.jpg', 0, 2),
(2, 'd:\\wamp\\www\\wingman\\uploads\\img_1_1.jpg', 0, 1),
(3, 'd:\\wamp\\www\\wingman\\uploads\\img_1_2.jpg', 0, 1),
(4, 'd:\\wamp\\www\\wingman\\uploads\\img_1_3.jpg', 0, 1),
(8, 'C:\\wamp64\\www\\wingman\\uploads\\img_101_0.jpg', 0, 101),
(11, 'C:\\wamp64\\www\\wingman\\uploads\\img_101_1.jpg', 0, 101);

-- --------------------------------------------------------

--
-- Table structure for table `userprofiles`
--

CREATE TABLE `userprofiles` (
  `id` int(11) NOT NULL,
  `userId` int(11) NOT NULL,
  `firstName` text COLLATE utf8_bin,
  `lastName` text COLLATE utf8_bin,
  `birthday` text COLLATE utf8_bin,
  `city` text COLLATE utf8_bin,
  `state` text COLLATE utf8_bin,
  `zip` text COLLATE utf8_bin
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_bin;

--
-- Dumping data for table `userprofiles`
--

INSERT INTO `userprofiles` (`id`, `userId`, `firstName`, `lastName`, `birthday`, `city`, `state`, `zip`) VALUES
(1, 1, 'FName0', 'LName0', NULL, NULL, NULL, NULL),
(2, 2, 'snake', 'LName1', 'null', NULL, NULL, NULL),
(3, 3, 'FName2', 'LName2', NULL, NULL, NULL, NULL),
(4, 4, 'FName3', 'LName3', NULL, NULL, NULL, NULL),
(5, 5, 'FName4', 'LName4', NULL, NULL, NULL, NULL),
(6, 6, 'FName5', 'LName5', NULL, NULL, NULL, NULL),
(7, 7, 'FName6', 'LName6', NULL, NULL, NULL, NULL),
(8, 8, 'FName7', 'LName7', NULL, NULL, NULL, NULL),
(9, 9, 'FName8', 'LName8', NULL, NULL, NULL, NULL),
(10, 10, 'FName9', 'LName9', NULL, NULL, NULL, NULL),
(11, 11, 'FName10', 'LName10', NULL, NULL, NULL, NULL),
(12, 12, 'FName11', 'LName11', NULL, NULL, NULL, NULL),
(13, 13, 'FName12', 'LName12', NULL, NULL, NULL, NULL),
(14, 14, 'FName13', 'LName13', NULL, NULL, NULL, NULL),
(15, 15, 'FName14', 'LName14', NULL, NULL, NULL, NULL),
(16, 16, 'FName15', 'LName15', NULL, NULL, NULL, NULL),
(17, 17, 'FName16', 'LName16', NULL, NULL, NULL, NULL),
(18, 18, 'FName17', 'LName17', NULL, NULL, NULL, NULL),
(19, 19, 'FName18', 'LName18', NULL, NULL, NULL, NULL),
(20, 20, 'FName19', 'LName19', NULL, NULL, NULL, NULL),
(21, 21, 'FName20', 'LName20', NULL, NULL, NULL, NULL),
(22, 22, 'FName21', 'LName21', NULL, NULL, NULL, NULL),
(23, 23, 'FName22', 'LName22', NULL, NULL, NULL, NULL),
(24, 24, 'FName23', 'LName23', NULL, NULL, NULL, NULL),
(25, 25, 'FName24', 'LName24', NULL, NULL, NULL, NULL),
(26, 26, 'FName25', 'LName25', NULL, NULL, NULL, NULL),
(27, 27, 'FName26', 'LName26', NULL, NULL, NULL, NULL),
(28, 28, 'FName27', 'LName27', NULL, NULL, NULL, NULL),
(29, 29, 'FName28', 'LName28', NULL, NULL, NULL, NULL),
(30, 30, 'FName29', 'LName29', NULL, NULL, NULL, NULL),
(31, 31, 'FName30', 'LName30', NULL, NULL, NULL, NULL),
(32, 32, 'FName31', 'LName31', NULL, NULL, NULL, NULL),
(33, 33, 'FName32', 'LName32', NULL, NULL, NULL, NULL),
(34, 34, 'FName33', 'LName33', NULL, NULL, NULL, NULL),
(35, 35, 'FName34', 'LName34', NULL, NULL, NULL, NULL),
(36, 36, 'FName35', 'LName35', NULL, NULL, NULL, NULL),
(37, 37, 'FName36', 'LName36', NULL, NULL, NULL, NULL),
(38, 38, 'FName37', 'LName37', NULL, NULL, NULL, NULL),
(39, 39, 'FName38', 'LName38', NULL, NULL, NULL, NULL),
(40, 40, 'FName39', 'LName39', NULL, NULL, NULL, NULL),
(41, 41, 'FName40', 'LName40', NULL, NULL, NULL, NULL),
(42, 42, 'FName41', 'LName41', NULL, NULL, NULL, NULL),
(43, 43, 'FName42', 'LName42', NULL, NULL, NULL, NULL),
(44, 44, 'FName43', 'LName43', NULL, NULL, NULL, NULL),
(45, 45, 'FName44', 'LName44', NULL, NULL, NULL, NULL),
(46, 46, 'FName45', 'LName45', NULL, NULL, NULL, NULL),
(47, 47, 'FName46', 'LName46', NULL, NULL, NULL, NULL),
(48, 48, 'FName47', 'LName47', NULL, NULL, NULL, NULL),
(49, 49, 'FName48', 'LName48', NULL, NULL, NULL, NULL),
(50, 50, 'FName49', 'LName49', NULL, NULL, NULL, NULL),
(51, 51, 'FName50', 'LName50', NULL, NULL, NULL, NULL),
(52, 52, 'FName51', 'LName51', NULL, NULL, NULL, NULL),
(53, 53, 'FName52', 'LName52', NULL, NULL, NULL, NULL),
(54, 54, 'FName53', 'LName53', NULL, NULL, NULL, NULL),
(55, 55, 'FName54', 'LName54', NULL, NULL, NULL, NULL),
(56, 56, 'FName55', 'LName55', NULL, NULL, NULL, NULL),
(57, 57, 'FName56', 'LName56', NULL, NULL, NULL, NULL),
(58, 58, 'FName57', 'LName57', NULL, NULL, NULL, NULL),
(59, 59, 'FName58', 'LName58', NULL, NULL, NULL, NULL),
(60, 60, 'FName59', 'LName59', NULL, NULL, NULL, NULL),
(61, 61, 'FName60', 'LName60', NULL, NULL, NULL, NULL),
(62, 62, 'FName61', 'LName61', NULL, NULL, NULL, NULL),
(63, 63, 'FName62', 'LName62', NULL, NULL, NULL, NULL),
(64, 64, 'FName63', 'LName63', NULL, NULL, NULL, NULL),
(65, 65, 'FName64', 'LName64', NULL, NULL, NULL, NULL),
(66, 66, 'FName65', 'LName65', NULL, NULL, NULL, NULL),
(67, 67, 'FName66', 'LName66', NULL, NULL, NULL, NULL),
(68, 68, 'FName67', 'LName67', NULL, NULL, NULL, NULL),
(69, 69, 'FName68', 'LName68', NULL, NULL, NULL, NULL),
(70, 70, 'FName69', 'LName69', NULL, NULL, NULL, NULL),
(71, 71, 'FName70', 'LName70', NULL, NULL, NULL, NULL),
(72, 72, 'FName71', 'LName71', NULL, NULL, NULL, NULL),
(73, 73, 'FName72', 'LName72', NULL, NULL, NULL, NULL),
(74, 74, 'FName73', 'LName73', NULL, NULL, NULL, NULL),
(75, 75, 'FName74', 'LName74', NULL, NULL, NULL, NULL),
(76, 76, 'FName75', 'LName75', NULL, NULL, NULL, NULL),
(77, 77, 'FName76', 'LName76', NULL, NULL, NULL, NULL),
(78, 78, 'FName77', 'LName77', NULL, NULL, NULL, NULL),
(79, 79, 'FName78', 'LName78', NULL, NULL, NULL, NULL),
(80, 80, 'FName79', 'LName79', NULL, NULL, NULL, NULL),
(81, 81, 'FName80', 'LName80', NULL, NULL, NULL, NULL),
(82, 82, 'FName81', 'LName81', NULL, NULL, NULL, NULL),
(83, 83, 'FName82', 'LName82', NULL, NULL, NULL, NULL),
(84, 84, 'FName83', 'LName83', NULL, NULL, NULL, NULL),
(85, 85, 'FName84', 'LName84', NULL, NULL, NULL, NULL),
(86, 86, 'FName85', 'LName85', NULL, NULL, NULL, NULL),
(87, 87, 'FName86', 'LName86', NULL, NULL, NULL, NULL),
(88, 88, 'FName87', 'LName87', NULL, NULL, NULL, NULL),
(89, 89, 'FName88', 'LName88', NULL, NULL, NULL, NULL),
(90, 90, 'FName89', 'LName89', NULL, NULL, NULL, NULL),
(91, 91, 'FName90', 'LName90', NULL, NULL, NULL, NULL),
(92, 92, 'FName91', 'LName91', NULL, NULL, NULL, NULL),
(93, 93, 'FName92', 'LName92', NULL, NULL, NULL, NULL),
(94, 94, 'FName93', 'LName93', NULL, NULL, NULL, NULL),
(95, 95, 'FName94', 'LName94', NULL, NULL, NULL, NULL),
(96, 96, 'FName95', 'LName95', NULL, NULL, NULL, NULL),
(97, 97, 'FName96', 'LName96', NULL, NULL, NULL, NULL),
(98, 98, 'FName97', 'LName97', NULL, NULL, NULL, NULL),
(99, 99, 'FName98', 'LName98', NULL, NULL, NULL, NULL),
(100, 100, 'FName99', 'LName99', NULL, NULL, NULL, NULL),
(101, 101, 'name1', 'lastname2', '10/10/1980', NULL, NULL, NULL);

-- --------------------------------------------------------

--
-- Table structure for table `users`
--

CREATE TABLE `users` (
  `id` int(10) UNSIGNED NOT NULL,
  `email` text COLLATE utf8_bin NOT NULL,
  `password` text COLLATE utf8_bin NOT NULL,
  `sessionId` varchar(100) COLLATE utf8_bin DEFAULT NULL,
  `lastActivity` datetime DEFAULT NULL,
  `currentCheckInId` int(11) DEFAULT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_bin;

--
-- Dumping data for table `users`
--

INSERT INTO `users` (`id`, `email`, `password`, `sessionId`, `lastActivity`, `currentCheckInId`) VALUES
(1, 'email0@gmail.com', '*196BDEDE2AE4F84CA44C47D54D78478C7E2BD7B7', '0dj6oba97sdqdlqbvibcimr7t3', NULL, 15),
(2, 'email1@gmail.com', '*196BDEDE2AE4F84CA44C47D54D78478C7E2BD7B7', 'ctf4grraes7v1ek1anf78v3uh0', NULL, 21),
(3, 'email2@gmail.com', '*196BDEDE2AE4F84CA44C47D54D78478C7E2BD7B7', NULL, NULL, 3),
(4, 'email3@gmail.com', '*196BDEDE2AE4F84CA44C47D54D78478C7E2BD7B7', NULL, NULL, 4),
(5, 'email4@gmail.com', '*196BDEDE2AE4F84CA44C47D54D78478C7E2BD7B7', NULL, NULL, 5),
(6, 'email5@gmail.com', '*196BDEDE2AE4F84CA44C47D54D78478C7E2BD7B7', NULL, NULL, 6),
(7, 'email6@gmail.com', '*196BDEDE2AE4F84CA44C47D54D78478C7E2BD7B7', NULL, NULL, 7),
(8, 'email7@gmail.com', '*196BDEDE2AE4F84CA44C47D54D78478C7E2BD7B7', NULL, NULL, 8),
(9, 'email8@gmail.com', '*196BDEDE2AE4F84CA44C47D54D78478C7E2BD7B7', NULL, NULL, NULL),
(10, 'email9@gmail.com', '*196BDEDE2AE4F84CA44C47D54D78478C7E2BD7B7', NULL, NULL, NULL),
(11, 'email10@gmail.com', '*196BDEDE2AE4F84CA44C47D54D78478C7E2BD7B7', NULL, NULL, NULL),
(12, 'email11@gmail.com', '*196BDEDE2AE4F84CA44C47D54D78478C7E2BD7B7', NULL, NULL, NULL),
(13, 'email12@gmail.com', '*196BDEDE2AE4F84CA44C47D54D78478C7E2BD7B7', NULL, NULL, NULL),
(14, 'email13@gmail.com', '*196BDEDE2AE4F84CA44C47D54D78478C7E2BD7B7', NULL, NULL, NULL),
(15, 'email14@gmail.com', '*196BDEDE2AE4F84CA44C47D54D78478C7E2BD7B7', NULL, NULL, NULL),
(16, 'email15@gmail.com', '*196BDEDE2AE4F84CA44C47D54D78478C7E2BD7B7', NULL, NULL, NULL),
(17, 'email16@gmail.com', '*196BDEDE2AE4F84CA44C47D54D78478C7E2BD7B7', NULL, NULL, NULL),
(18, 'email17@gmail.com', '*196BDEDE2AE4F84CA44C47D54D78478C7E2BD7B7', NULL, NULL, NULL),
(19, 'email18@gmail.com', '*196BDEDE2AE4F84CA44C47D54D78478C7E2BD7B7', NULL, NULL, NULL),
(20, 'email19@gmail.com', '*196BDEDE2AE4F84CA44C47D54D78478C7E2BD7B7', NULL, NULL, NULL),
(21, 'email20@gmail.com', '*196BDEDE2AE4F84CA44C47D54D78478C7E2BD7B7', NULL, NULL, NULL),
(22, 'email21@gmail.com', '*196BDEDE2AE4F84CA44C47D54D78478C7E2BD7B7', NULL, NULL, NULL),
(23, 'email22@gmail.com', '*196BDEDE2AE4F84CA44C47D54D78478C7E2BD7B7', NULL, NULL, NULL),
(24, 'email23@gmail.com', '*196BDEDE2AE4F84CA44C47D54D78478C7E2BD7B7', NULL, NULL, NULL),
(25, 'email24@gmail.com', '*196BDEDE2AE4F84CA44C47D54D78478C7E2BD7B7', NULL, NULL, NULL),
(26, 'email25@gmail.com', '*196BDEDE2AE4F84CA44C47D54D78478C7E2BD7B7', NULL, NULL, NULL),
(27, 'email26@gmail.com', '*196BDEDE2AE4F84CA44C47D54D78478C7E2BD7B7', NULL, NULL, NULL),
(28, 'email27@gmail.com', '*196BDEDE2AE4F84CA44C47D54D78478C7E2BD7B7', NULL, NULL, NULL),
(29, 'email28@gmail.com', '*196BDEDE2AE4F84CA44C47D54D78478C7E2BD7B7', NULL, NULL, NULL),
(30, 'email29@gmail.com', '*196BDEDE2AE4F84CA44C47D54D78478C7E2BD7B7', NULL, NULL, NULL),
(31, 'email30@gmail.com', '*196BDEDE2AE4F84CA44C47D54D78478C7E2BD7B7', NULL, NULL, NULL),
(32, 'email31@gmail.com', '*196BDEDE2AE4F84CA44C47D54D78478C7E2BD7B7', NULL, NULL, NULL),
(33, 'email32@gmail.com', '*196BDEDE2AE4F84CA44C47D54D78478C7E2BD7B7', NULL, NULL, NULL),
(34, 'email33@gmail.com', '*196BDEDE2AE4F84CA44C47D54D78478C7E2BD7B7', NULL, NULL, NULL),
(35, 'email34@gmail.com', '*196BDEDE2AE4F84CA44C47D54D78478C7E2BD7B7', NULL, NULL, NULL),
(36, 'email35@gmail.com', '*196BDEDE2AE4F84CA44C47D54D78478C7E2BD7B7', NULL, NULL, NULL),
(37, 'email36@gmail.com', '*196BDEDE2AE4F84CA44C47D54D78478C7E2BD7B7', NULL, NULL, NULL),
(38, 'email37@gmail.com', '*196BDEDE2AE4F84CA44C47D54D78478C7E2BD7B7', NULL, NULL, NULL),
(39, 'email38@gmail.com', '*196BDEDE2AE4F84CA44C47D54D78478C7E2BD7B7', NULL, NULL, NULL),
(40, 'email39@gmail.com', '*196BDEDE2AE4F84CA44C47D54D78478C7E2BD7B7', NULL, NULL, NULL),
(41, 'email40@gmail.com', '*196BDEDE2AE4F84CA44C47D54D78478C7E2BD7B7', NULL, NULL, NULL),
(42, 'email41@gmail.com', '*196BDEDE2AE4F84CA44C47D54D78478C7E2BD7B7', NULL, NULL, NULL),
(43, 'email42@gmail.com', '*196BDEDE2AE4F84CA44C47D54D78478C7E2BD7B7', NULL, NULL, NULL),
(44, 'email43@gmail.com', '*196BDEDE2AE4F84CA44C47D54D78478C7E2BD7B7', NULL, NULL, NULL),
(45, 'email44@gmail.com', '*196BDEDE2AE4F84CA44C47D54D78478C7E2BD7B7', NULL, NULL, NULL),
(46, 'email45@gmail.com', '*196BDEDE2AE4F84CA44C47D54D78478C7E2BD7B7', NULL, NULL, NULL),
(47, 'email46@gmail.com', '*196BDEDE2AE4F84CA44C47D54D78478C7E2BD7B7', NULL, NULL, NULL),
(48, 'email47@gmail.com', '*196BDEDE2AE4F84CA44C47D54D78478C7E2BD7B7', NULL, NULL, NULL),
(49, 'email48@gmail.com', '*196BDEDE2AE4F84CA44C47D54D78478C7E2BD7B7', NULL, NULL, NULL),
(50, 'email49@gmail.com', '*196BDEDE2AE4F84CA44C47D54D78478C7E2BD7B7', NULL, NULL, NULL),
(51, 'email50@gmail.com', '*196BDEDE2AE4F84CA44C47D54D78478C7E2BD7B7', NULL, NULL, NULL),
(52, 'email51@gmail.com', '*196BDEDE2AE4F84CA44C47D54D78478C7E2BD7B7', NULL, NULL, NULL),
(53, 'email52@gmail.com', '*196BDEDE2AE4F84CA44C47D54D78478C7E2BD7B7', NULL, NULL, NULL),
(54, 'email53@gmail.com', '*196BDEDE2AE4F84CA44C47D54D78478C7E2BD7B7', NULL, NULL, NULL),
(55, 'email54@gmail.com', '*196BDEDE2AE4F84CA44C47D54D78478C7E2BD7B7', NULL, NULL, NULL),
(56, 'email55@gmail.com', '*196BDEDE2AE4F84CA44C47D54D78478C7E2BD7B7', NULL, NULL, NULL),
(57, 'email56@gmail.com', '*196BDEDE2AE4F84CA44C47D54D78478C7E2BD7B7', NULL, NULL, NULL),
(58, 'email57@gmail.com', '*196BDEDE2AE4F84CA44C47D54D78478C7E2BD7B7', NULL, NULL, NULL),
(59, 'email58@gmail.com', '*196BDEDE2AE4F84CA44C47D54D78478C7E2BD7B7', NULL, NULL, NULL),
(60, 'email59@gmail.com', '*196BDEDE2AE4F84CA44C47D54D78478C7E2BD7B7', NULL, NULL, NULL),
(61, 'email60@gmail.com', '*196BDEDE2AE4F84CA44C47D54D78478C7E2BD7B7', NULL, NULL, NULL),
(62, 'email61@gmail.com', '*196BDEDE2AE4F84CA44C47D54D78478C7E2BD7B7', NULL, NULL, NULL),
(63, 'email62@gmail.com', '*196BDEDE2AE4F84CA44C47D54D78478C7E2BD7B7', NULL, NULL, NULL),
(64, 'email63@gmail.com', '*196BDEDE2AE4F84CA44C47D54D78478C7E2BD7B7', NULL, NULL, NULL),
(65, 'email64@gmail.com', '*196BDEDE2AE4F84CA44C47D54D78478C7E2BD7B7', NULL, NULL, 20),
(66, 'email65@gmail.com', '*196BDEDE2AE4F84CA44C47D54D78478C7E2BD7B7', NULL, NULL, NULL),
(67, 'email66@gmail.com', '*196BDEDE2AE4F84CA44C47D54D78478C7E2BD7B7', NULL, NULL, NULL),
(68, 'email67@gmail.com', '*196BDEDE2AE4F84CA44C47D54D78478C7E2BD7B7', NULL, NULL, NULL),
(69, 'email68@gmail.com', '*196BDEDE2AE4F84CA44C47D54D78478C7E2BD7B7', NULL, NULL, NULL),
(70, 'email69@gmail.com', '*196BDEDE2AE4F84CA44C47D54D78478C7E2BD7B7', NULL, NULL, NULL),
(71, 'email70@gmail.com', '*196BDEDE2AE4F84CA44C47D54D78478C7E2BD7B7', NULL, NULL, NULL),
(72, 'email71@gmail.com', '*196BDEDE2AE4F84CA44C47D54D78478C7E2BD7B7', NULL, NULL, NULL),
(73, 'email72@gmail.com', '*196BDEDE2AE4F84CA44C47D54D78478C7E2BD7B7', NULL, NULL, NULL),
(74, 'email73@gmail.com', '*196BDEDE2AE4F84CA44C47D54D78478C7E2BD7B7', NULL, NULL, NULL),
(75, 'email74@gmail.com', '*196BDEDE2AE4F84CA44C47D54D78478C7E2BD7B7', NULL, NULL, NULL),
(76, 'email75@gmail.com', '*196BDEDE2AE4F84CA44C47D54D78478C7E2BD7B7', NULL, NULL, NULL),
(77, 'email76@gmail.com', '*196BDEDE2AE4F84CA44C47D54D78478C7E2BD7B7', NULL, NULL, NULL),
(78, 'email77@gmail.com', '*196BDEDE2AE4F84CA44C47D54D78478C7E2BD7B7', NULL, NULL, NULL),
(79, 'email78@gmail.com', '*196BDEDE2AE4F84CA44C47D54D78478C7E2BD7B7', NULL, NULL, NULL),
(80, 'email79@gmail.com', '*196BDEDE2AE4F84CA44C47D54D78478C7E2BD7B7', NULL, NULL, NULL),
(81, 'email80@gmail.com', '*196BDEDE2AE4F84CA44C47D54D78478C7E2BD7B7', NULL, NULL, NULL),
(82, 'email81@gmail.com', '*196BDEDE2AE4F84CA44C47D54D78478C7E2BD7B7', NULL, NULL, NULL),
(83, 'email82@gmail.com', '*196BDEDE2AE4F84CA44C47D54D78478C7E2BD7B7', NULL, NULL, NULL),
(84, 'email83@gmail.com', '*196BDEDE2AE4F84CA44C47D54D78478C7E2BD7B7', NULL, NULL, NULL),
(85, 'email84@gmail.com', '*196BDEDE2AE4F84CA44C47D54D78478C7E2BD7B7', NULL, NULL, NULL),
(86, 'email85@gmail.com', '*196BDEDE2AE4F84CA44C47D54D78478C7E2BD7B7', NULL, NULL, NULL),
(87, 'email86@gmail.com', '*196BDEDE2AE4F84CA44C47D54D78478C7E2BD7B7', NULL, NULL, NULL),
(88, 'email87@gmail.com', '*196BDEDE2AE4F84CA44C47D54D78478C7E2BD7B7', NULL, NULL, NULL),
(89, 'email88@gmail.com', '*196BDEDE2AE4F84CA44C47D54D78478C7E2BD7B7', NULL, NULL, NULL),
(90, 'email89@gmail.com', '*196BDEDE2AE4F84CA44C47D54D78478C7E2BD7B7', NULL, NULL, NULL),
(91, 'email90@gmail.com', '*196BDEDE2AE4F84CA44C47D54D78478C7E2BD7B7', NULL, NULL, NULL),
(92, 'email91@gmail.com', '*196BDEDE2AE4F84CA44C47D54D78478C7E2BD7B7', NULL, NULL, NULL),
(93, 'email92@gmail.com', '*196BDEDE2AE4F84CA44C47D54D78478C7E2BD7B7', NULL, NULL, NULL),
(94, 'email93@gmail.com', '*196BDEDE2AE4F84CA44C47D54D78478C7E2BD7B7', NULL, NULL, NULL),
(95, 'email94@gmail.com', '*196BDEDE2AE4F84CA44C47D54D78478C7E2BD7B7', NULL, NULL, NULL),
(96, 'email95@gmail.com', '*196BDEDE2AE4F84CA44C47D54D78478C7E2BD7B7', NULL, NULL, NULL),
(97, 'email96@gmail.com', '*196BDEDE2AE4F84CA44C47D54D78478C7E2BD7B7', NULL, NULL, NULL),
(98, 'email97@gmail.com', '*196BDEDE2AE4F84CA44C47D54D78478C7E2BD7B7', NULL, NULL, NULL),
(99, 'email98@gmail.com', '*196BDEDE2AE4F84CA44C47D54D78478C7E2BD7B7', NULL, NULL, NULL),
(100, 'email99@gmail.com', '*196BDEDE2AE4F84CA44C47D54D78478C7E2BD7B7', NULL, NULL, NULL),
(101, 'email100@gmail.com', '*196BDEDE2AE4F84CA44C47D54D78478C7E2BD7B7', 'tp814gdbqup48d0hvgic1lv674', NULL, 19);

--
-- Indexes for dumped tables
--

--
-- Indexes for table `inboxes`
--
ALTER TABLE `inboxes`
  ADD PRIMARY KEY (`id`);

--
-- Indexes for table `inboxmessages`
--
ALTER TABLE `inboxmessages`
  ADD PRIMARY KEY (`id`);

--
-- Indexes for table `usercheckins`
--
ALTER TABLE `usercheckins`
  ADD PRIMARY KEY (`id`);

--
-- Indexes for table `userphotos`
--
ALTER TABLE `userphotos`
  ADD PRIMARY KEY (`id`);

--
-- Indexes for table `userprofiles`
--
ALTER TABLE `userprofiles`
  ADD PRIMARY KEY (`id`);

--
-- Indexes for table `users`
--
ALTER TABLE `users`
  ADD PRIMARY KEY (`id`),
  ADD UNIQUE KEY `uniqueemail` (`email`(200)),
  ADD KEY `sessionId` (`sessionId`);

--
-- AUTO_INCREMENT for dumped tables
--

--
-- AUTO_INCREMENT for table `inboxes`
--
ALTER TABLE `inboxes`
  MODIFY `id` int(10) UNSIGNED NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=2;
--
-- AUTO_INCREMENT for table `inboxmessages`
--
ALTER TABLE `inboxmessages`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT;
--
-- AUTO_INCREMENT for table `usercheckins`
--
ALTER TABLE `usercheckins`
  MODIFY `id` int(10) UNSIGNED NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=22;
--
-- AUTO_INCREMENT for table `userphotos`
--
ALTER TABLE `userphotos`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=12;
--
-- AUTO_INCREMENT for table `userprofiles`
--
ALTER TABLE `userprofiles`
  MODIFY `id` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=102;
--
-- AUTO_INCREMENT for table `users`
--
ALTER TABLE `users`
  MODIFY `id` int(10) UNSIGNED NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=102;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
