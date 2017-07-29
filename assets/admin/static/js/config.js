var changepwd_username = '';
var delete_users = [];
var new_users = [];
var original_users = [];

function submit() {
    $('#save-failure').css('display', 'none');
    $('#save-success').css('display', 'none');
    $.post('/admin/config/edit', JSON.stringify({
        'site-name': $('#site-name').val(),
        'site-url': $('#site-url').val(),
        'site-description': $('#site-description').val(),
        'ip': $('#ip').val(),
        'port': $('#port').val(),
        'theme': $('#theme').val(),
        'users': {
            'new': new_users,
            'delete': delete_users
        }
    }), function(data, status, xhr) {
        if (data == 'success') {
            $('#save-success').css('display', 'block');
        } else {
            $('#save-failure').css('display', 'block');
        }
    }, 'text');
}

function is_original(username) {
    for (var i = 0; i < original_users.length; ++i) {
        if (original_users[i] === username) {
            return true;
        }
    }
    return false;
}

function change_password() {
    for (var i = 0; i < new_users.length; ++i) {
        if (new_users[i].username === changepwd_username) {
            new_users[i].password = $('#new-password').val();
            $('#modal-changepwd').modal('hide');
            return;
        }
    }
    for (var i = 0; i < delete_users.length; ++i) {
        if (delete_users[i] === changepwd_username) {
            return;
        }
    }
    if (is_original(changepwd_username)) {
        delete_users.push(changepwd_username);
    }
    new_users.push({
        'username': changepwd_username,
        'password': $('#new-password').val()
    });
    $('#modal-changepwd').modal('hide');
}

function delete_admin() {
    var delete_username = decodeURI($(this).attr('data-username'));
    for (var i = 0; i < new_users.length; ++i) {
        if (new_users[i].username === delete_username) {
            new_users.splice(i, 1);
        }
    }
    for (var i = 0; i < delete_users.length; ++i) {
        if (delete_users[i] === changepwd_username) {
            $('tr[data-username="' + $(this).attr('data-username') + '"]').remove();
            return;
        }
    }
    if (is_original(delete_username)) {
        delete_users.push(delete_username);
    }
    $('tr[data-username="' + $(this).attr('data-username') + '"]').remove();
}

function create_admin() {
    var username = $('#username').val();
    var password = $('#password').val();
    if (is_original(username)) {
        var is_delete = false;
        for (var i = 0; i < delete_users.length; ++i) {
            if (delete_users[i] === username) {
                is_delete = true;
                break;
            }
        }
        if (!is_delete) {
            $('#newadmin-failure').css('display', 'block');
            return;
        }
    }
    for (var i = 0; i < new_users.length; ++i) {
        if (new_users[i].username === username) {
            $('#newadmin-failure').css('display', 'block');
            return;
        }
    }
    new_users.push({
        'username': username,
        'password': password
    });
    $('#users').append('<tr data-username="' + encodeURI(username) + '"><td class="col-md-8">' + username +
        '</td><td class="col-md-2"><a data-username="' + encodeURI(username) +
        '" class="change-password">Change password</a></td><td class="col-md-2"><a data-username="' + encodeURI(username) +
        '" class="delete-admin">Delete</a></td></tr>');
    $('.change-password').click(function () {
        changepwd_username = decodeURI($(this).attr('data-username'));
        $('#new-password').val('');
        $('#modal-changepwd').modal('show');
    });
    $('.delete-admin').click(delete_admin);
    $('#modal-newadmin').modal('hide');
}

$(document).ready(function () {
    $('#submit').click(submit);
    $('.change-password').click(function () {
        changepwd_username = decodeURI($(this).attr('data-username'));
        $('#new-password').val('');
        $('#modal-changepwd').modal('show');
    });
    $('.delete-admin').click(delete_admin);
    $('#new-admin').click(function () {
        $('#username').val('');
        $('#password').val('');
        $('#modal-newadmin').modal('show');
        $('#newadmin-failure').css('display', 'none');
    });
    $('#create-admin').click(create_admin);
    $('#change-password').click(change_password);
});