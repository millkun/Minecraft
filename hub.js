const mineflayer = require('mineflayer');
const { pathfinder, Movements, goals } = require('mineflayer-pathfinder');
const fs = require('fs');
const path = require('path');
const TelegramBot = require('node-telegram-bot-api');

const bot = mineflayer.createBot({
  host: 'PLAY.TESLACRAFT.ORG',
  port: 25565,
  username: 'Morkovka935',
  timeout: 240000
});

bot.loadPlugin(pathfinder);

const developerNick = '_Morkovka_';
const logFilePath = path.join(__dirname, 'parkour_log.txt');

// Координаты паркура
const parkourPoints = [
  { x: 2110, y: 206, z: 6843 }, // Начало паркура
  { x: 2103, y: 206, z: 6841 },
  { x: 2097, y: 208, z: 6842 },
  { x: 2094, y: 209, z: 6843 },
  { x: 2092, y: 210, z: 6846 },
  { x: 2091, y: 210, z: 6849 },
  { x: 2092, y: 210, z: 6853 },
  { x: 2096, y: 210, z: 6860 },
  { x: 2102, y: 193, z: 6867 },
  { x: 2109, y: 193, z: 6870 },
  { x: 2116, y: 193, z: 6867 },
  { x: 2115, y: 193, z: 6863 },
  { x: 2116, y: 193, z: 6860 },
  { x: 2117, y: 191, z: 6858 },
  { x: 2118, y: 191, z: 6854 },
  { x: 2117, y: 192, z: 6851 },
  { x: 2112, y: 194, z: 6848 },
  { x: 2110, y: 195, z: 6850 },
  { x: 2104, y: 196, z: 6860 },
  { x: 2104, y: 196, z: 6860 } // Конец паркура
];

// Структура для отслеживания прогресса игроков
const playerProgress = {};

// Игроки рядом с ботом
const playersNearBot = new Set();

// Функция для загрузки данных из лог-файла
function loadParkourLog() {
  if (fs.existsSync(logFilePath)) {
    const data = fs.readFileSync(logFilePath, 'utf-8');
    return data.split(' \n').map(line => line.trim()).filter(line => line).map(line => {
      const [username, completions, bestTime] = line.split(' ');
      return { username, completions: parseInt(completions, 10), bestTime: parseFloat(bestTime) };
    });
  }
  return [];
}

// Функция для сохранения данных в лог-файл
function saveParkourLog(logData) {
  const data = logData.map(entry => `${entry.username} ${entry.completions} ${entry.bestTime}`).join(' \n');
  fs.writeFileSync(logFilePath, data);
}

let parkourLog = loadParkourLog();

// Инициализация Telegram бота
const telegramToken = '7800594114:AAFt78t1RgkAPrfES9sMZ5wMVO2rXqbPwZw';
const telegramBot = new TelegramBot(telegramToken, { polling: true });
const chatId = '945518256'; // ID чата, в который будут отправляться сообщения

bot.once('spawn', function () {
  console.log('Bot has spawned');
  const defaultMove = new Movements(bot);
  bot.pathfinder.setMovements(defaultMove);

  function lookAtNearestPlayer() {
    const players = bot.players;
    let nearestPlayer = null;
    let nearestDistance = Infinity;

    for (const playerName in players) {
      if (playerName === bot.username) continue;
      const player = players[playerName];
      if (player.entity && player.entity.position) {
        const distance = bot.entity.position.distanceTo(player.entity.position);
        if (distance < nearestDistance && distance < 10) {
          nearestDistance = distance;
          nearestPlayer = player;
        }
      }
    }

    if (nearestPlayer) {
      const playerPosition = nearestPlayer.entity.position.offset(0, 1.62, 0);
      bot.lookAt(playerPosition);
    }
  }

  setInterval(lookAtNearestPlayer, 10);

  setTimeout(() => {
    bot.chat(`/Survival`);
  }, 40000);

  // Функция для формирования и отправки топа игроков
  function sendTopPlayers() {
    const updatedLog = loadParkourLog(); // Загружаем актуальные данные из лог-файла
    const topPlayers = updatedLog
      .filter(log => log.bestTime < Infinity) // Убираем игроков без завершений
      .sort((a, b) => a.bestTime - b.bestTime) // Сортируем по времени
      .slice(0, 5); // Берем только топ-5

    if (topPlayers.length > 0) {
      topPlayers.forEach((log, index) => {
        const message = `${index + 2} &f${index + 1}. &c&l${log.username} &f[&4&l${log.completions}&f] - &e${log.bestTime.toFixed(2)} &fсекунд`;
        setTimeout(() => {
          bot.chat(`/holo line stats ${message}`);
        }, index * 1500); // Задержка в 1.5 секунды для каждого последующего сообщения
      });
      setTimeout(() => {
        bot.chat('/holo line stats 7 &6Обновление каждые &a5 минут&6, если бот онлайн');
      }, topPlayers.length * 1500); // Задержка в 1.5 секунды после последнего сообщения из топа
    } else {
      bot.chat('Произошла непредвиденая ошибка');
      telegramBot.sendMessage(chatId, 'Произошла непредвиденая ошибка при обновлении топа. #ошибки');
    }
  }

  // Устанавливаем интервал для отправки топа каждые 5 минут
  setInterval(sendTopPlayers, 5 * 60 * 1000);

  // Устанавливаем интервал для отправки сообщения в чат каждые полчаса
  setInterval(() => {
    bot.chat('!&e&l/&6&lw&e&la&6&lr&e&lp &6&lh&e&lu&6&lb &f&l- Интерактивное &c&lа&4&lф&c&lк&4&l-&c&lл&4&lо&c&lб&4&lб&c&lи &f&l для игроков');
    telegramBot.sendMessage(chatId, 'Агитационное сообщение отправлено. #агитация');
  }, 30 * 60 * 1000);
});

bot.on('playerJoined', (player) => {
  console.log(`${player.username} joined the server.`);
  playerProgress[player.username] = { currentPoint: 0, startTime: null };
});

bot.on('entityMoved', (entity) => {
  if (entity.type === 'player' && entity.username) {
    const playerName = entity.username;
    const playerPos = entity.position;
    const distanceToBot = bot.entity.position.distanceTo(playerPos);

    if (distanceToBot < 100) {
      if (!playersNearBot.has(playerName)) {
        playersNearBot.add(playerName);
        telegramBot.sendMessage(chatId, `${playerName} зашёл в хаб. #онлайн`);
      }
    } else {
      if (playersNearBot.has(playerName)) {
        playersNearBot.delete(playerName);
        telegramBot.sendMessage(chatId, `${playerName} покинул хаб. #онлайн`);
      }
    }

    if (playerProgress[playerName]) {
      const currentPoint = playerProgress[playerName].currentPoint;
      const currentParkourPoint = parkourPoints[currentPoint];

      const distanceToPoint = playerPos.distanceTo(currentParkourPoint);
      const distanceToStart = playerPos.distanceTo(parkourPoints[0]);
      if (distanceToPoint < 1.5) { // Проверка на близость к точке
        if (currentPoint === 0 && playerProgress[playerName].startTime === null) {
          playerProgress[playerName].startTime = new Date();
        }
        playerProgress[playerName].currentPoint++;
        console.log(`Player ${playerName} at point ${playerProgress[playerName].currentPoint}`);

        if (playerProgress[playerName].currentPoint === parkourPoints.length - 1) {
          const endTime = new Date();
          const timeTaken = (endTime - playerProgress[playerName].startTime) / 1000;
          console.log(`${playerName} completed the parkour in ${timeTaken} seconds!`);
          bot.chat(`/pay ${playerName} 20 Прохождение мини-паркура`);
          bot.chat(`/pay confirm`);
          bot.chat(`&8-░-░-░-░-░-░-░-░-░-░-░-░-░-░-░ &6&l[&e&lАфк-Лобби&6&l] &a&l${playerName} &bпрошёл мини-паркур за &e${timeTaken.toFixed(2)} &bсекунд и получил &e$&a20`);

          let playerLog = parkourLog.find(log => log.username === playerName);
          if (!playerLog) {
            playerLog = { username: playerName, completions: 0, bestTime: Infinity };
            parkourLog.push(playerLog);
          }
          playerLog.completions++;
          if (timeTaken < playerLog.bestTime) {
            playerLog.bestTime = timeTaken;
          }
          saveParkourLog(parkourLog);

          // Сброс прогресса после завершения паркура
          playerProgress[playerName].currentPoint = 0;
          playerProgress[playerName].startTime = null;
          console.log(`Progress reset after completion for ${playerName}`);

          // Отправка сообщения в Telegram
          telegramBot.sendMessage(chatId, `${playerName} прошёл паркур за ${timeTaken.toFixed(2)} секунд! #паркур`);
        }
      } else if (currentPoint === 0 && distanceToStart < 1.5) {
        // Сброс прогресса, если игрок пересекает стартовую точку
        playerProgress[playerName] = { currentPoint: 0, startTime: null };
        console.log(`Progress deleted [start] for ${playerName}`);
      }
    }
  }
});

// Запись данных вручную
function handleReturnCommand(username, commandParts) {
  if (commandParts.length !== 3) {
    bot.chat(`/m ${username} Неверный формат команды. Используйте: -return <ник> <значение>`);
    return;
  }

  const targetUsername = commandParts[1];
  const value = parseFloat(commandParts[2]);

  if (isNaN(value)) {
    bot.chat(`/m ${username} Значение должно быть числом.`);
    return;
  }

  // Ищем игрока, игнорируя регистр
  let playerLog = parkourLog.find(log => log.username.toLowerCase() === targetUsername.toLowerCase());
  
  // Если игрок не найден, создаем новую запись
  if (!playerLog) {
    playerLog = { username: targetUsername, completions: 0, bestTime: Infinity };
    parkourLog.push(playerLog);
  }

  // Обновляем данные
  if (Number.isInteger(value)) {
    playerLog.completions = value;
    bot.chat(`/m ${username} Количество прохождений для ${playerLog.username} изменено на ${value}.`);
  } else {
    playerLog.bestTime = value;
    bot.chat(`/m ${username} Лучшее время для ${playerLog.username} изменено на ${value.toFixed(3)} секунд.`);
  }

  saveParkourLog(parkourLog);
}

// Функция для удаления цветного форматирования
function removeColorFormatting(text) {
  return text.replace(/(&[0-9a-fk-or]|§[0-9a-fk-or])/g, '');
}

bot.on('chat', (username, message) => {
  if (username === bot.username) return; // Игнорируем сообщения от самого бота

  // Убираем цветное форматирование из сообщения
  const cleanMessage = removeColorFormatting(message);
  
  console.log(`${username} -> ${cleanMessage}`);
  telegramBot.sendMessage(chatId, `${username} -> ${cleanMessage}`);

  if (username === developerNick) {
    const commandParts = cleanMessage.toLowerCase().split(' ');
    const command = commandParts[0];
    switch (command) {
      case '-here':
        bot.chat('Телепортируюсь к Вам, мой господин');
        bot.chat('/tpa _Morkovka_');
        break;
      case '-holo':
        bot.chat('/holo line stats 8 &2(c) &amillkun &2- Разработчик лобби &a_Morkovka_');
        break;
      case '-return':
        handleReturnCommand(username, commandParts);
        break;
      case '-stats':
        bot.chat('Вывел полную статистику в консоль, хозяин');
        console.log('Parkour Statistics:');
        parkourLog.forEach(log => {
          console.log(`${log.username}: ${log.completions} completions, best time: ${log.bestTime.toFixed(2)} seconds`);
        });
        break;
    }
  } else if (username === developerNick || username !== developerNick) {
    const command = cleanMessage.toLowerCase(); // Используем очищенное сообщение
    switch (command) {
      case '-?':
        bot.chat(`/m ${username} Это интерактивное афк-лобби. Тут можно проходить мини-паркур и регистрироваться на ивенты. По мини-паркуру есть топ игроков. Варп - hub`);
        break;
    }
  }
});

bot.on('error', (err) => {
  console.error('Error:', err);
  telegramBot.sendMessage(chatId, `Ошибка: ${err.message}. #ошибки`);
});

bot.on('end', () => {
  console.log('Bot has disconnected.');
  telegramBot.sendMessage(chatId, 'Бот отключился от сервера. #ошибки');
});
